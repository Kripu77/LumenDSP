#pragma once

#include <JuceHeader.h>

namespace lumen::audio
{

class ReverbFx
{
public:
    void prepare(const juce::dsp::ProcessSpec& processSpec);
    void reset();
    void setEnabled(bool shouldEnable) noexcept;
    void setRoomSize(float size01) noexcept;
    void setDamping(float damping01) noexcept;
    void setMix(float mix01) noexcept;
    void process(juce::AudioBuffer<float>& buffer);

private:
    void applyParameters();

    juce::dsp::Reverb reverb;
    juce::dsp::Reverb::Parameters params{};
    bool enabled = false;
    bool prepared = false;
    float mix = 0.18f;
};

} // namespace lumen::audio
