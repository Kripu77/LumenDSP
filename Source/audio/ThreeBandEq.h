#pragma once

#include <JuceHeader.h>

namespace lumen::audio
{

class ThreeBandEq
{
public:
    void prepare(const juce::dsp::ProcessSpec& processSpec);
    void reset();
    void setEnabled(bool shouldEnable);
    void setBassGainDb(float gainDb);
    void setMidGainDb(float gainDb);
    void setTrebleGainDb(float gainDb);
    void process(juce::AudioBuffer<float>& buffer);

private:
    static constexpr float bassCenterFrequencyHertz = 100.0f;
    static constexpr float midCenterFrequencyHertz = 750.0f;
    static constexpr float trebleCenterFrequencyHertz = 4000.0f;
    static constexpr float bassQualityFactor = 0.707f;
    static constexpr float midQualityFactor = 0.9f;
    static constexpr float trebleQualityFactor = 0.707f;
    static constexpr float gainUpdateSmoothingSeconds = 0.02f;

    void updateFiltersIfNeeded();

    using Filter = juce::dsp::IIR::Filter<float>;
    using Coefficients = juce::dsp::IIR::Coefficients<float>;
    using DuplicatedFilter = juce::dsp::ProcessorDuplicator<Filter, Coefficients>;

    juce::dsp::ProcessSpec currentSpec{};
    DuplicatedFilter bassFilter;
    DuplicatedFilter midFilter;
    DuplicatedFilter trebleFilter;

    bool enabled = true;
    bool prepared = false;
    float bassGainDb = 0.0f;
    float midGainDb = 0.0f;
    float trebleGainDb = 0.0f;
    float appliedBassGainDb = 0.0f;
    float appliedMidGainDb = 0.0f;
    float appliedTrebleGainDb = 0.0f;
};

} // namespace lumen::audio
