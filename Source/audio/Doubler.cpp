#include "audio/Doubler.h"

namespace lumen::audio
{

void Doubler::prepare(const juce::dsp::ProcessSpec& processSpec)
{
    sampleRate = processSpec.sampleRate > 0.0 ? processSpec.sampleRate : 48000.0;
    line.setMaximumDelayInSamples(static_cast<int>(sampleRate * (maximumSpreadMs + 2.0f * driftMs) * 0.001) + 8);
    line.prepare({ sampleRate, processSpec.maximumBlockSize, 1 });
    spreadSamples.reset(sampleRate, 0.1);
    reset();
}

void Doubler::reset()
{
    line.reset();
    driftPhase = 0.0f;
}

void Doubler::setEnabled(bool shouldEnable) noexcept
{
    if (shouldEnable && !enabled)
        reset();
    enabled = shouldEnable;
}

void Doubler::setSpreadMs(float spreadMs) noexcept
{
    spreadSamples.setTargetValue(static_cast<float>(sampleRate * juce::jlimit(3.0f, maximumSpreadMs, spreadMs) * 0.001));
}

void Doubler::process(juce::AudioBuffer<float>& buffer)
{
    if (!enabled || buffer.getNumChannels() < 2)
        return;

    const float twoPi = juce::MathConstants<float>::twoPi;
    const float driftIncrement = twoPi * 0.23f / static_cast<float>(sampleRate);
    const float driftSamples = driftMs * 0.001f * static_cast<float>(sampleRate);
    auto* left = buffer.getWritePointer(0);
    auto* right = buffer.getWritePointer(1);

    for (int i = 0; i < buffer.getNumSamples(); ++i)
    {
        driftPhase = std::fmod(driftPhase + driftIncrement, twoPi);
        const float centre = 0.5f * (left[i] + right[i]);
        line.pushSample(0, centre);
        left[i] = centre;
        right[i] = line.popSample(0, spreadSamples.getNextValue() + driftSamples * (1.0f + std::sin(driftPhase)));
    }
}

} // namespace lumen::audio
