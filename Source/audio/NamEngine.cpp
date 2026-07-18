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
    currentSampleRateHertz = sampleRateHertz > 0.0 ? sampleRateHertz : 44100.0;
    currentMaximumBlockSizeSamples = juce::jmax(1, maximumBlockSizeSamples);

    inputScratch.assign(static_cast<size_t>(currentMaximumBlockSizeSamples), static_cast<NAM_SAMPLE>(0));
    outputScratch.assign(static_cast<size_t>(currentMaximumBlockSizeSamples), static_cast<NAM_SAMPLE>(0));
    inputPointers[monoChannelIndex] = inputScratch.data();
    outputPointers[monoChannelIndex] = outputScratch.data();

    prepared = true;

    std::lock_guard<std::mutex> lock(modelMutex);

    if (activeModel != nullptr)
        activeModel->Reset(currentSampleRateHertz, currentMaximumBlockSizeSamples);

    if (stagedModel != nullptr)
        stagedModel->Reset(currentSampleRateHertz, currentMaximumBlockSizeSamples);
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
        lastErrorMessage = "Model file does not exist.";
        return false;
    }

    modelLoading.store(true, std::memory_order_relaxed);

    try
    {
        nam::DspLoadOptions loadOptions;
        loadOptions.prewarm = true;

        auto loadedModel = nam::get_dsp(
            std::filesystem::path(modelFile.getFullPathName().toStdString()),
            loadOptions);

        if (loadedModel == nullptr)
        {
            lastErrorMessage = "NeuralAmpModelerCore returned an empty model.";
            modelLoading.store(false, std::memory_order_relaxed);
            return false;
        }

        if (prepared)
            loadedModel->Reset(currentSampleRateHertz, currentMaximumBlockSizeSamples);

        {
            std::lock_guard<std::mutex> lock(modelMutex);
            stagedModel = std::move(loadedModel);
            stagedModelFile = modelFile;
            stagedModelName = modelFile.getFileName();
            lastErrorMessage.clear();
            stagedModelReady.store(true, std::memory_order_release);
        }

        modelLoading.store(false, std::memory_order_relaxed);
        return true;
    }
    catch (const std::exception& exception)
    {
        lastErrorMessage = juce::String("Failed to load model: ") + exception.what();
        modelLoading.store(false, std::memory_order_relaxed);
        return false;
    }
    catch (...)
    {
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
    stagedModelReady.store(false, std::memory_order_relaxed);
    modelLoaded.store(false, std::memory_order_relaxed);
    activeModelFile = juce::File();
    activeModelName.clear();
}

void NamEngine::process(juce::AudioBuffer<float>& buffer)
{
    applyStagedModel();

    if (!prepared || !modelLoaded.load(std::memory_order_relaxed))
        return;

    nam::DSP* modelPointer = nullptr;

    {
        std::lock_guard<std::mutex> lock(modelMutex);
        modelPointer = activeModel.get();
    }

    if (modelPointer == nullptr)
        return;

    const int sampleCount = buffer.getNumSamples();
    const int channelCount = buffer.getNumChannels();

    if (sampleCount <= 0 || channelCount <= 0)
        return;

    if (sampleCount > currentMaximumBlockSizeSamples)
        return;

    float* monoChannel = buffer.getWritePointer(monoChannelIndex);
    processMonoThroughModel(*modelPointer, monoChannel, sampleCount);

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

void NamEngine::applyStagedModel()
{
    if (!stagedModelReady.load(std::memory_order_acquire))
        return;

    std::lock_guard<std::mutex> lock(modelMutex);

    if (!stagedModelReady.load(std::memory_order_relaxed) || stagedModel == nullptr)
        return;

    activeModel = std::move(stagedModel);
    activeModelFile = stagedModelFile;
    activeModelName = stagedModelName;
    stagedModelReady.store(false, std::memory_order_relaxed);
    modelLoaded.store(true, std::memory_order_relaxed);
}

void NamEngine::processMonoThroughModel(
    nam::DSP& model,
    float* channelData,
    int sampleCount)
{
    for (int sampleIndex = 0; sampleIndex < sampleCount; ++sampleIndex)
        inputScratch[static_cast<size_t>(sampleIndex)] = static_cast<NAM_SAMPLE>(channelData[sampleIndex]);

    model.process(inputPointers, outputPointers, sampleCount);

    for (int sampleIndex = 0; sampleIndex < sampleCount; ++sampleIndex)
        channelData[sampleIndex] = static_cast<float>(outputScratch[static_cast<size_t>(sampleIndex)]);
}

} // namespace lumen::audio
