#include "audio/Delay.h"

namespace lumen::audio
{

void Delay::prepare(const juce::dsp::ProcessSpec& processSpec)
{
    sampleRate = processSpec.sampleRate > 0.0 ? processSpec.sampleRate : 48000.0;
    const int maxSamples = juce::jmax(
        1,
        static_cast<int>(sampleRate * maximumDelayMs * 0.001) + 8);
    const int channels = juce::jmax(1, static_cast<int>(processSpec.numChannels));

    delayLines.assign(static_cast<size_t>(channels), std::vector<float>(static_cast<size_t>(maxSamples), 0.0f));
    writeIndex = 0;
    prepared = true;
    setTimeMs(380.0f);
    reset();
}

void Delay::reset()
{
    for (auto& line : delayLines)
        std::fill(line.begin(), line.end(), 0.0f);
    writeIndex = 0;
}

void Delay::setEnabled(bool shouldEnable) noexcept
{
    enabled = shouldEnable;
}

void Delay::setTimeMs(float timeMs) noexcept
{
    const float clamped = juce::jlimit(1.0f, maximumDelayMs, timeMs);
    delaySamples = juce::jmax(1, static_cast<int>(sampleRate * clamped * 0.001));
    if (!delayLines.empty())
        delaySamples = juce::jmin(delaySamples, static_cast<int>(delayLines[0].size()) - 1);
}

void Delay::setFeedback(float feedback01) noexcept
{
    feedback = juce::jlimit(0.0f, 0.92f, feedback01);
}

void Delay::setMix(float mix01) noexcept
{
    mix = juce::jlimit(0.0f, 1.0f, mix01);
}

void Delay::process(juce::AudioBuffer<float>& buffer)
{
    if (!prepared || !enabled || delayLines.empty())
        return;

    const int channels = juce::jmin(buffer.getNumChannels(), static_cast<int>(delayLines.size()));
    const int samples = buffer.getNumSamples();
    const int lineSize = static_cast<int>(delayLines[0].size());

    for (int i = 0; i < samples; ++i)
    {
        const int readIndex = (writeIndex - delaySamples + lineSize) % lineSize;

        for (int ch = 0; ch < channels; ++ch)
        {
            float* dry = buffer.getWritePointer(ch);
            auto& line = delayLines[static_cast<size_t>(ch)];
            const float delayed = line[static_cast<size_t>(readIndex)];
            const float input = dry[i];
            line[static_cast<size_t>(writeIndex)] = input + delayed * feedback;
            dry[i] = input * (1.0f - mix) + delayed * mix;
        }

        writeIndex = (writeIndex + 1) % lineSize;
    }
}

} // namespace lumen::audio
