#include "audio/Compressor.h"

namespace lumen::audio
{

void Compressor::prepare(const juce::dsp::ProcessSpec& processSpec)
{
    compressor.prepare(processSpec);
    compressor.setAttack(12.0f);
    compressor.setRelease(120.0f);
    dryBuffer.setSize(
        static_cast<int>(processSpec.numChannels),
        static_cast<int>(processSpec.maximumBlockSize),
        false,
        true,
        true);
    prepared = true;
    reset();
}

void Compressor::reset()
{
    compressor.reset();
}

void Compressor::setEnabled(bool shouldEnable) noexcept
{
    enabled = shouldEnable;
}

void Compressor::setThresholdDb(float thresholdDb) noexcept
{
    compressor.setThreshold(thresholdDb);
}

void Compressor::setRatio(float ratio) noexcept
{
    compressor.setRatio(juce::jmax(1.0f, ratio));
}

void Compressor::setMix(float mix01) noexcept
{
    mix = juce::jlimit(0.0f, 1.0f, mix01);
}

void Compressor::process(juce::AudioBuffer<float>& buffer)
{
    if (!prepared || !enabled)
        return;

    const int channels = buffer.getNumChannels();
    const int samples = buffer.getNumSamples();
    dryBuffer.makeCopyOf(buffer, true);

    juce::dsp::AudioBlock<float> block(buffer);
    juce::dsp::ProcessContextReplacing<float> context(block);
    compressor.process(context);

    for (int ch = 0; ch < channels; ++ch)
    {
        auto* wet = buffer.getWritePointer(ch);
        const auto* dry = dryBuffer.getReadPointer(ch);
        for (int i = 0; i < samples; ++i)
            wet[i] = dry[i] * (1.0f - mix) + wet[i] * mix;
    }
}

} // namespace lumen::audio
