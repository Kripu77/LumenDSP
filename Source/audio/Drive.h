#pragma once

#include <JuceHeader.h>

namespace lumen::audio
{

class Drive
{
public:
    void prepare(const juce::dsp::ProcessSpec& processSpec);
    void reset();
    void setEnabled(bool shouldEnable) noexcept;
    void setDrive(float drive01) noexcept;
    void setTone(float tone01) noexcept;
    void setLevel(float level01) noexcept;
    void process(juce::AudioBuffer<float>& buffer);

private:
    void updateToneFilter();

    using Filter = juce::dsp::IIR::Filter<float>;
    using Coefficients = juce::dsp::IIR::Coefficients<float>;
    using DuplicatedFilter = juce::dsp::ProcessorDuplicator<Filter, Coefficients>;

    juce::dsp::ProcessSpec currentSpec{};
    DuplicatedFilter toneFilter;
    bool enabled = false;
    bool prepared = false;
    float drive = 0.35f;
    float tone = 0.5f;
    float level = 0.7f;
    float appliedTone = -1.0f;
};

} // namespace lumen::audio
