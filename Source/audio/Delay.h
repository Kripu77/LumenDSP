#pragma once

#include <JuceHeader.h>
#include <vector>

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
    void process(juce::AudioBuffer<float>& buffer);

    float getEffectiveTimeMs() const noexcept;

private:
    static constexpr float maximumDelayMs = 2000.0f;

    void updateDelaySamples();
    static float beatsForDivision(DelayDivision division);

    double sampleRate = 48000.0;
    int writeIndex = 0;
    int delaySamples = 1;
    std::vector<std::vector<float>> delayLines;
    bool enabled = false;
    bool prepared = false;
    bool syncEnabled = false;
    DelayDivision division = DelayDivision::quarter;
    float manualTimeMs = 380.0f;
    float tempoBpm = 120.0f;
    float feedback = 0.25f;
    float mix = 0.2f;
};

} // namespace lumen::audio
