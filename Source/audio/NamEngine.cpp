#include "audio/NamEngine.h"
#include <NAM/activations.h>
#include <NAM/container.h>
#include <NAM/convnet.h>
#include <NAM/linear.h>
#include <NAM/lstm.h>
#include <NAM/wavenet/model.h>
#include <filesystem>

namespace lumen::audio
{

namespace
{

using NamConfigFactory = std::unique_ptr<nam::ModelConfig> (*)(const nlohmann::json&, double);

[[gnu::used]] NamConfigFactory forceLinkedWaveNetFactory = &nam::wavenet::create_config;
[[gnu::used]] NamConfigFactory forceLinkedLstmFactory = &nam::lstm::create_config;
[[gnu::used]] NamConfigFactory forceLinkedConvNetFactory = &nam::convnet::create_config;
[[gnu::used]] NamConfigFactory forceLinkedLinearFactory = &nam::linear::create_config;
[[gnu::used]] NamConfigFactory forceLinkedContainerFactory = &nam::container::create_config;

} // namespace

NamEngine::NamEngine()
{
    nam::activations::Activation::enable_fast_tanh();
}

NamEngine::~NamEngine() = default;

void NamEngine::prepare(double sampleRateHertz, int maximumBlockSizeSamples)
{
    const double safeRate = sampleRateHertz > 0.0 ? sampleRateHertz : 44100.0;
    const int safeBlock = juce::jmax(1, maximumBlockSizeSamples);

    std::lock_guard<std::mutex> lock(modelMutex);

    currentSampleRateHertz = safeRate;
    currentMaximumBlockSizeSamples = safeBlock;

    inputScratch.assign(static_cast<size_t>(currentMaximumBlockSizeSamples), static_cast<NAM_SAMPLE>(0));
    outputScratch.assign(static_cast<size_t>(currentMaximumBlockSizeSamples), static_cast<NAM_SAMPLE>(0));
    inputPointers[monoChannelIndex] = inputScratch.data();
    outputPointers[monoChannelIndex] = outputScratch.data();

    if (activeModel != nullptr)
        activeModel->Reset(currentSampleRateHertz, currentMaximumBlockSizeSamples);

    if (stagedModel != nullptr)
        stagedModel->Reset(currentSampleRateHertz, currentMaximumBlockSizeSamples);

    prepared.store(true, std::memory_order_release);
}

void NamEngine::reset()
{
    std::lock_guard<std::mutex> lock(modelMutex);

    if (activeModel != nullptr)
        activeModel->Reset(currentSampleRateHertz, currentMaximumBlockSizeSamples);
}

bool NamEngine::loadModelAsync(const juce::File& modelFile)
{
    if (!modelFile.existsAsFile())
    {
        std::lock_guard<std::mutex> lock(modelMutex);
        lastErrorMessage = "Model file does not exist: " + modelFile.getFullPathName();
        modelLoading.store(false, std::memory_order_relaxed);
        return false;
    }

    modelLoading.store(true, std::memory_order_relaxed);

    try
    {
        // Skip prewarm so load stays responsive; first notes may be slightly cold.
        nam::DspLoadOptions loadOptions;
        loadOptions.prewarm = false;

        auto loadedModel = nam::get_dsp(
            std::filesystem::path(modelFile.getFullPathName().toStdString()),
            loadOptions);

        if (loadedModel == nullptr)
        {
            std::lock_guard<std::mutex> lock(modelMutex);
            lastErrorMessage = "NeuralAmpModelerCore returned an empty model.";
            modelLoading.store(false, std::memory_order_relaxed);
            return false;
        }

        // Own as shared_ptr before Reset / publish so audio can take a ref safely.
        ModelPtr readyModel(std::move(loadedModel));

        {
            std::lock_guard<std::mutex> lock(modelMutex);

            if (prepared.load(std::memory_order_relaxed))
                readyModel->Reset(currentSampleRateHertz, currentMaximumBlockSizeSamples);

            // Stage only — promote on the audio thread (or immediately below under lock
            // into active as a shared_ptr). Never destroy a model the audio thread may
            // still be processing: process() holds its own shared_ptr copy.
            stagedModel = std::move(readyModel);
            stagedModelFile = modelFile;
            stagedModelName = modelFile.getFileName();
            lastErrorMessage.clear();

            // Promote under the same lock so UI sees a loaded model immediately,
            // without racing process(): process copies activeModel first.
            promoteStagedModelLocked();
        }

        modelLoading.store(false, std::memory_order_relaxed);
        return true;
    }
    catch (const std::exception& exception)
    {
        std::lock_guard<std::mutex> lock(modelMutex);
        lastErrorMessage = juce::String("Failed to load model: ") + exception.what();
        modelLoading.store(false, std::memory_order_relaxed);
        return false;
    }
    catch (...)
    {
        std::lock_guard<std::mutex> lock(modelMutex);
        lastErrorMessage = "Failed to load model for an unknown reason.";
        modelLoading.store(false, std::memory_order_relaxed);
        return false;
    }
}

void NamEngine::clearModel()
{
    std::lock_guard<std::mutex> lock(modelMutex);
    stagedModel.reset();
    activeModel.reset();
    stagedModelFile = juce::File();
    stagedModelName.clear();
    activeModelFile = juce::File();
    activeModelName.clear();
    stagedModelReady.store(false, std::memory_order_relaxed);
    modelLoaded.store(false, std::memory_order_relaxed);
}

void NamEngine::process(juce::AudioBuffer<float>& buffer)
{
    if (!prepared.load(std::memory_order_acquire))
        return;

    // Take a local shared_ptr under the lock. Background loads may replace
    // activeModel after this; the local ref keeps the old model alive for this block.
    ModelPtr model;
    int maxBlock = 0;

    {
        std::lock_guard<std::mutex> lock(modelMutex);
        promoteStagedModelLocked();
        model = activeModel;
        maxBlock = currentMaximumBlockSizeSamples;
    }

    if (model == nullptr)
        return;

    const int sampleCount = buffer.getNumSamples();
    const int channelCount = buffer.getNumChannels();

    if (sampleCount <= 0 || channelCount <= 0 || sampleCount > maxBlock)
        return;

    // Scratch is only resized under modelMutex in prepare(). JUCE does not call
    // processBlock concurrently with prepareToPlay; still bound-check for safety.
    if (static_cast<int>(inputScratch.size()) < sampleCount
        || static_cast<int>(outputScratch.size()) < sampleCount)
        return;

    float* monoChannel = buffer.getWritePointer(monoChannelIndex);
    processMonoThroughModel(*model, monoChannel, sampleCount);

    for (int channelIndex = 1; channelIndex < channelCount; ++channelIndex)
        buffer.copyFrom(channelIndex, 0, monoChannel, sampleCount);
}

bool NamEngine::isModelLoaded() const noexcept
{
    return modelLoaded.load(std::memory_order_relaxed);
}

bool NamEngine::isModelLoading() const noexcept
{
    return modelLoading.load(std::memory_order_relaxed);
}

juce::String NamEngine::getLoadedModelName() const
{
    std::lock_guard<std::mutex> lock(modelMutex);
    return activeModelName;
}

juce::File NamEngine::getLoadedModelFile() const
{
    std::lock_guard<std::mutex> lock(modelMutex);
    return activeModelFile;
}

juce::String NamEngine::getLastErrorMessage() const
{
    std::lock_guard<std::mutex> lock(modelMutex);
    return lastErrorMessage;
}

void NamEngine::promoteStagedModelLocked()
{
    if (stagedModel == nullptr)
        return;

    activeModel = std::move(stagedModel);
    activeModelFile = stagedModelFile;
    activeModelName = stagedModelName;
    stagedModel.reset();
    stagedModelReady.store(false, std::memory_order_relaxed);
    modelLoaded.store(true, std::memory_order_release);
}

void NamEngine::processMonoThroughModel(
    nam::DSP& model,
    float* channelData,
    int sampleCount)
{
    for (int sampleIndex = 0; sampleIndex < sampleCount; ++sampleIndex)
        inputScratch[static_cast<size_t>(sampleIndex)] = static_cast<NAM_SAMPLE>(channelData[sampleIndex]);

    // Re-bind pointers in case vectors reallocated (prepare grows under lock only).
    inputPointers[monoChannelIndex] = inputScratch.data();
    outputPointers[monoChannelIndex] = outputScratch.data();

    model.process(inputPointers, outputPointers, sampleCount);

    for (int sampleIndex = 0; sampleIndex < sampleCount; ++sampleIndex)
        channelData[sampleIndex] = static_cast<float>(outputScratch[static_cast<size_t>(sampleIndex)]);
}

} // namespace lumen::audio
