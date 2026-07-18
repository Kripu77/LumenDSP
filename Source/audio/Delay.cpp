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

    delayLines.assign(
        static_cast<size_t>(channels),
        std::vector<float>(static_cast<size_t>(maxSamples), 0.0f));
    writeIndex = 0;
    prepared = true;
    updateDelaySamples();
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
    manualTimeMs = juce::jlimit(1.0f, maximumDelayMs, timeMs);
    if (!syncEnabled)
        updateDelaySamples();
}

void Delay::setSyncEnabled(bool shouldSync) noexcept
{
    syncEnabled = shouldSync;
    updateDelaySamples();
}

void Delay::setDivision(DelayDivision newDivision) noexcept
{
    division = newDivision;
    if (syncEnabled)
        updateDelaySamples();
}

void Delay::setTempoBpm(float bpm) noexcept
{
    tempoBpm = juce::jlimit(40.0f, 240.0f, bpm);
    if (syncEnabled)
        updateDelaySamples();
}

void Delay::setFeedback(float feedback01) noexcept
{
    feedback = juce::jlimit(0.0f, 0.92f, feedback01);
}

void Delay::setMix(float mix01) noexcept
{
    mix = juce::jlimit(0.0f, 1.0f, mix01);
}

float Delay::getEffectiveTimeMs() const noexcept
{
    if (!syncEnabled)
        return manualTimeMs;

    const float beats = beatsForDivision(division);
    return (60000.0f / tempoBpm) * beats;
}

float Delay::beatsForDivision(DelayDivision value)
{
    switch (value)
    {
        case DelayDivision::eighth: return 0.5f;
        case DelayDivision::eighthDotted: return 0.75f;
        case DelayDivision::sixteenth: return 0.25f;
        case DelayDivision::quarterDotted: return 1.5f;
        case DelayDivision::half: return 2.0f;
        case DelayDivision::quarter:
        default: return 1.0f;
    }
}

void Delay::updateDelaySamples()
{
    const float timeMs = juce::jlimit(1.0f, maximumDelayMs, getEffectiveTimeMs());
    delaySamples = juce::jmax(1, static_cast<int>(sampleRate * timeMs * 0.001));
    if (!delayLines.empty())
        delaySamples = juce::jmin(delaySamples, static_cast<int>(delayLines[0].size()) - 1);
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
