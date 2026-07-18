#pragma once

#include <JuceHeader.h>
#include <vector>

namespace lumen::audio
{

class Delay
{
public:
    void prepare(const juce::dsp::ProcessSpec& processSpec);
    void reset();
    void setEnabled(bool shouldEnable) noexcept;
    void setTimeMs(float timeMs) noexcept;
    void setFeedback(float feedback01) noexcept;
    void setMix(float mix01) noexcept;
    void process(juce::AudioBuffer<float>& buffer);

private:
    static constexpr float maximumDelayMs = 2000.0f;

    double sampleRate = 48000.0;
    int writeIndex = 0;
    int delaySamples = 1;
    std::vector<std::vector<float>> delayLines;
    bool enabled = false;
    bool prepared = false;
    float feedback = 0.25f;
    float mix = 0.2f;
};

} // namespace lumen::audio
