#include "audio/IrConvolver.h"

namespace lumen::audio
{

IrConvolver::IrConvolver() = default;

IrConvolver::~IrConvolver() = default;

void IrConvolver::prepare(const juce::dsp::ProcessSpec& processSpec)
{
    currentSpec = processSpec;
    convolution.prepare(processSpec);
    monoScratchBuffer.setSize(
        static_cast<int>(processSpec.numChannels),
        static_cast<int>(processSpec.maximumBlockSize),
        false,
        true,
        true);
    dryScratchBuffer.setSize(
        static_cast<int>(processSpec.numChannels),
        static_cast<int>(processSpec.maximumBlockSize),
        false,
        true,
        true);
    prepared = true;
    reset();
}

void IrConvolver::reset()
{
    convolution.reset();
}

void IrConvolver::setEnabled(bool shouldEnable)
{
    enabled.store(shouldEnable, std::memory_order_relaxed);
}

void IrConvolver::setMix(float wetAmount)
{
    mixWet.store(juce::jlimit(0.0f, 1.0f, wetAmount), std::memory_order_relaxed);
}

bool IrConvolver::loadImpulseResponseAsync(const juce::File& impulseResponseFile)
{
    if (!impulseResponseFile.existsAsFile())
        return false;

    juce::AudioFormatManager formatManager;
    formatManager.registerBasicFormats();

    std::unique_ptr<juce::AudioFormatReader> reader(
        formatManager.createReaderFor(impulseResponseFile));

    if (reader == nullptr)
        return false;

    const auto sampleCount = static_cast<int>(
        juce::jmin(reader->lengthInSamples, static_cast<juce::int64>(maximumImpulseResponseLengthSamples)));
    const auto channelCount = static_cast<int>(juce::jmin<juce::uint32>(reader->numChannels, 2u));

    if (sampleCount <= 0 || channelCount <= 0)
        return false;

    juce::AudioBuffer<float> impulseBuffer(channelCount, sampleCount);
    reader->read(&impulseBuffer, 0, sampleCount, 0, true, true);

    {
        std::lock_guard<std::mutex> lock(loadMutex);
        pendingImpulseResponse = std::move(impulseBuffer);
        pendingSampleRateHertz = reader->sampleRate;
        pendingFile = impulseResponseFile;
        pendingLoadReady.store(true, std::memory_order_release);
    }

    return true;
}

void IrConvolver::clearImpulseResponse()
{
    std::lock_guard<std::mutex> lock(loadMutex);
    pendingImpulseResponse.setSize(0, 0);
    pendingFile = juce::File();
    pendingLoadReady.store(false, std::memory_order_release);
    loaded.store(false, std::memory_order_relaxed);
    activeFile = juce::File();
    activeFileName.clear();
    convolution.reset();
}

void IrConvolver::process(juce::AudioBuffer<float>& buffer)
{
    applyPendingImpulseResponse();

    if (!enabled.load(std::memory_order_relaxed) || !loaded.load(std::memory_order_relaxed) || !prepared)
        return;

    const float wet = mixWet.load(std::memory_order_relaxed);
    const float dry = 1.0f - wet;
    const int channels = buffer.getNumChannels();
    const int samples = buffer.getNumSamples();

    if (dry > 1.0e-5f
        && dryScratchBuffer.getNumChannels() >= channels
        && dryScratchBuffer.getNumSamples() >= samples)
    {
        for (int ch = 0; ch < channels; ++ch)
            dryScratchBuffer.copyFrom(ch, 0, buffer, ch, 0, samples);
    }

    juce::dsp::AudioBlock<float> audioBlock(buffer);
    juce::dsp::ProcessContextReplacing<float> context(audioBlock);
    convolution.process(context);

    if (dry > 1.0e-5f
        && dryScratchBuffer.getNumChannels() >= channels
        && dryScratchBuffer.getNumSamples() >= samples)
    {
        for (int ch = 0; ch < channels; ++ch)
        {
            auto* wetData = buffer.getWritePointer(ch);
            const auto* dryData = dryScratchBuffer.getReadPointer(ch);
            for (int i = 0; i < samples; ++i)
                wetData[i] = dry * dryData[i] + wet * wetData[i];
        }
    }
}

bool IrConvolver::isLoaded() const noexcept
{
    return loaded.load(std::memory_order_relaxed);
}

juce::String IrConvolver::getLoadedFileName() const
{
    std::lock_guard<std::mutex> lock(loadMutex);
    return activeFileName;
}

juce::File IrConvolver::getLoadedFile() const
{
    std::lock_guard<std::mutex> lock(loadMutex);
    return activeFile;
}

void IrConvolver::applyPendingImpulseResponse()
{
    if (!pendingLoadReady.load(std::memory_order_acquire))
        return;

    juce::AudioBuffer<float> impulseToLoad;
    juce::File fileToActivate;
    double impulseSampleRateHertz = 0.0;

    {
        std::lock_guard<std::mutex> lock(loadMutex);

        if (!pendingLoadReady.load(std::memory_order_relaxed))
            return;

        impulseToLoad = std::move(pendingImpulseResponse);
        fileToActivate = pendingFile;
        impulseSampleRateHertz = pendingSampleRateHertz;
        pendingLoadReady.store(false, std::memory_order_relaxed);
    }

    if (impulseToLoad.getNumSamples() <= 0)
        return;

    convolution.loadImpulseResponse(
        std::move(impulseToLoad),
        impulseSampleRateHertz,
        juce::dsp::Convolution::Stereo::yes,
        juce::dsp::Convolution::Trim::yes,
        juce::dsp::Convolution::Normalise::yes);

    {
        std::lock_guard<std::mutex> lock(loadMutex);
        activeFile = fileToActivate;
        activeFileName = fileToActivate.getFileName();
        loaded.store(true, std::memory_order_relaxed);
    }
}

} // namespace lumen::audio
