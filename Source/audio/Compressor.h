#pragma once

#include <JuceHeader.h>

namespace lumen::audio
{

class Compressor
{
public:
    void prepare(const juce::dsp::ProcessSpec& processSpec);
    void reset();
    void setEnabled(bool shouldEnable) noexcept;
    void setThresholdDb(float thresholdDb) noexcept;
    void setRatio(float ratio) noexcept;
    void setMix(float mix01) noexcept;
    void process(juce::AudioBuffer<float>& buffer);

private:
    juce::dsp::Compressor<float> compressor;
    juce::AudioBuffer<float> dryBuffer;
    bool enabled = false;
    bool prepared = false;
    float mix = 1.0f;
};

} // namespace lumen::audio
