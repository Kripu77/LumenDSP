#pragma once

#include <JuceHeader.h>

namespace lumen::audio
{

// Widens a mono-sourced signal by playing a slightly delayed, drifting copy on the right side.
class Doubler
{
public:
    void prepare(const juce::dsp::ProcessSpec& processSpec);
    void reset();
    void setEnabled(bool shouldEnable) noexcept;
    void setSpreadMs(float spreadMs) noexcept;
    void process(juce::AudioBuffer<float>& buffer);

private:
    static constexpr float maximumSpreadMs = 20.0f;
    static constexpr float driftMs = 0.35f;

    juce::dsp::DelayLine<float, juce::dsp::DelayLineInterpolationTypes::Lagrange3rd> line;
    juce::SmoothedValue<float> spreadSamples;
    double sampleRate = 48000.0;
    float driftPhase = 0.0f;
    bool enabled = false;
};

} // namespace lumen::audio
