#pragma once

#include <JuceHeader.h>
#include <signalsmith-basics/reverb.h>

namespace lumen::audio
{

enum class ReverbCharacter
{
    room = 0,
    hall = 1,
    plate = 2,
    ambient = 3
};

// Stereo FDN reverb (Signalsmith Basics, MIT) with Plini-style decay, cut filters and width.
class ReverbFx
{
public:
    void prepare(const juce::dsp::ProcessSpec& processSpec);
    void reset();
    void setEnabled(bool shouldEnable) noexcept;
    void setCharacter(ReverbCharacter character) noexcept;
    void setDecaySeconds(float seconds) noexcept;
    void setLowCutHz(float hz) noexcept;
    void setHighCutHz(float hz) noexcept;
    void setWidth(float width01) noexcept;
    void setMix(float mix01) noexcept;
    void process(juce::AudioBuffer<float>& buffer);

private:
    signalsmith::basics::ReverbFloat reverb;
    juce::AudioBuffer<float> wet;
    juce::SmoothedValue<float> wetGain;
    juce::SmoothedValue<float> width;
    ReverbCharacter character = ReverbCharacter::hall;
    int maximumBlockSize = 0;
    bool enabled = false;
    bool prepared = false;
};

} // namespace lumen::audio
