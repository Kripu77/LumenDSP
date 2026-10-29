#include "audio/Delay.h"

namespace lumen::audio
{

void Delay::prepare(const juce::dsp::ProcessSpec& processSpec)
{
    sampleRate = processSpec.sampleRate > 0.0 ? processSpec.sampleRate : 48000.0;
    line.setMaximumDelayInSamples(
        static_cast<int>(sampleRate * (maximumDelayMs + 2.0f * maximumModulationMs) * 0.001) + 8);
    line.prepare(processSpec);
    toneFilter.prepare(processSpec);
    lowCutFilter.prepare(processSpec);
    lowCutFilter.setType(juce::dsp::FirstOrderTPTFilterType::highpass);
    lowCutFilter.setCutoffFrequency(90.0f);
    setTone(0.5f);
    delaySamples.reset(sampleRate, 0.3);
    prepared = true;
    updateDelaySamples();
    delaySamples.setCurrentAndTargetValue(delaySamples.getTargetValue());
    reset();
}

void Delay::reset()
{
    line.reset();
    toneFilter.reset();
    lowCutFilter.reset();
    wowPhase = 0.0f;
    flutterPhase = 0.0f;
}

void Delay::setEnabled(bool shouldEnable) noexcept
{
    // Clear stale repeats so re-enabling never replays old audio.
    if (shouldEnable && !enabled)
        reset();
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

void Delay::setTone(float tone01) noexcept
{
    toneFilter.setCutoffFrequency(1500.0f * std::pow(8.0f, juce::jlimit(0.0f, 1.0f, tone01)));
}

void Delay::setModulation(float depth01) noexcept
{
    modulation = juce::jlimit(0.0f, 1.0f, depth01);
}

void Delay::setSaturation(float amount01) noexcept
{
    saturation = juce::jlimit(0.0f, 1.0f, amount01);
}

void Delay::setPingPong(bool shouldPingPong) noexcept
{
    pingPong = shouldPingPong;
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
    delaySamples.setTargetValue(static_cast<float>(sampleRate * timeMs * 0.001));
}

void Delay::process(juce::AudioBuffer<float>& buffer)
{
    if (!prepared || !enabled)
        return;

    const int channels = juce::jmin(buffer.getNumChannels(), 2);
    const bool bounce = pingPong && channels == 2;
    const float twoPi = juce::MathConstants<float>::twoPi;
    const float wowIncrement = twoPi * 0.55f / static_cast<float>(sampleRate);
    const float flutterIncrement = twoPi * 5.3f / static_cast<float>(sampleRate);
    const float modulationSamples = modulation * maximumModulationMs * 0.001f * static_cast<float>(sampleRate);
    const float drive = 1.0f + 4.0f * saturation;

    for (int i = 0; i < buffer.getNumSamples(); ++i)
    {
        wowPhase = std::fmod(wowPhase + wowIncrement, twoPi);
        flutterPhase = std::fmod(flutterPhase + flutterIncrement, twoPi);
        const float baseDelay = delaySamples.getNextValue() + modulationSamples;

        float dry[2] {};
        float wet[2] {};
        for (int ch = 0; ch < channels; ++ch)
        {
            // Left and right wobble a quarter-cycle apart, which widens the repeats.
            const float offset = static_cast<float>(ch) * juce::MathConstants<float>::halfPi;
            const float wobble = 0.85f * std::sin(wowPhase + offset) + 0.15f * std::sin(flutterPhase + offset);
            dry[ch] = buffer.getSample(ch, i);
            const float repeat = line.popSample(ch, baseDelay + modulationSamples * wobble);
            wet[ch] = lowCutFilter.processSample(ch, toneFilter.processSample(ch, repeat));
        }

        for (int ch = 0; ch < channels; ++ch)
        {
            const float source = bounce ? wet[1 - ch] : wet[ch];
            const float tape = std::tanh(drive * source) / drive;
            const float input = bounce ? (ch == 0 ? 0.5f * (dry[0] + dry[1]) : 0.0f) : dry[ch];
            line.pushSample(ch, input + feedback * tape);
            buffer.setSample(ch, i, dry[ch] + mix * wet[ch]);
        }
    }
}

} // namespace lumen::audio
