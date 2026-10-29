#pragma once

#include <JuceHeader.h>

namespace lumen::audio
{

enum class DelayDivision
{
    quarter = 0,
    eighth = 1,
    eighthDotted = 2,
    sixteenth = 3,
    quarterDotted = 4,
    half = 5
};

// Stereo tape-style delay: wow/flutter, saturation and tone in the feedback loop, optional ping-pong.
class Delay
{
public:
    void prepare(const juce::dsp::ProcessSpec& processSpec);
    void reset();
    void setEnabled(bool shouldEnable) noexcept;
    void setTimeMs(float timeMs) noexcept;
    void setSyncEnabled(bool shouldSync) noexcept;
    void setDivision(DelayDivision division) noexcept;
    void setTempoBpm(float bpm) noexcept;
    void setFeedback(float feedback01) noexcept;
    void setMix(float mix01) noexcept;
    void setTone(float tone01) noexcept;
    void setModulation(float depth01) noexcept;
    void setSaturation(float amount01) noexcept;
    void setPingPong(bool shouldPingPong) noexcept;
    void process(juce::AudioBuffer<float>& buffer);

    float getEffectiveTimeMs() const noexcept;

private:
    static constexpr float maximumDelayMs = 3000.0f;
    static constexpr float maximumModulationMs = 3.0f;

    void updateDelaySamples();
    static float beatsForDivision(DelayDivision division);

    juce::dsp::DelayLine<float, juce::dsp::DelayLineInterpolationTypes::Lagrange3rd> line;
    juce::dsp::FirstOrderTPTFilter<float> toneFilter;
    juce::dsp::FirstOrderTPTFilter<float> lowCutFilter;
    juce::SmoothedValue<float> delaySamples;

    double sampleRate = 48000.0;
    float wowPhase = 0.0f;
    float flutterPhase = 0.0f;
    bool enabled = false;
    bool prepared = false;
    bool syncEnabled = false;
    bool pingPong = false;
    DelayDivision division = DelayDivision::quarter;
    float manualTimeMs = 380.0f;
    float tempoBpm = 120.0f;
    float feedback = 0.25f;
    float mix = 0.2f;
    float modulation = 0.3f;
    float saturation = 0.2f;
};

} // namespace lumen::audio
