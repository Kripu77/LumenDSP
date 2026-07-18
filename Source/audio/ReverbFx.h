#pragma once

#include <JuceHeader.h>

namespace lumen::audio
{

enum class ReverbCharacter
{
    room = 0,
    hall = 1,
    plate = 2,
    ambient = 3
};

class ReverbFx
{
public:
    void prepare(const juce::dsp::ProcessSpec& processSpec);
    void reset();
    void setEnabled(bool shouldEnable) noexcept;
    void setCharacter(ReverbCharacter character) noexcept;
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
    ReverbCharacter character = ReverbCharacter::room;
    float size = 0.4f;
    float damping = 0.5f;
    float mix = 0.18f;
};

} // namespace lumen::audio
