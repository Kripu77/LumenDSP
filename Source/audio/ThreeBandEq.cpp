#include "audio/ThreeBandEq.h"

namespace lumen::audio
{

namespace
{

constexpr float gainChangeEpsilonDb = 0.01f;

} // namespace

void ThreeBandEq::prepare(const juce::dsp::ProcessSpec& processSpec)
{
    currentSpec = processSpec;
    bassFilter.prepare(processSpec);
    midFilter.prepare(processSpec);
    trebleFilter.prepare(processSpec);
    prepared = true;
    updateFiltersIfNeeded();
    reset();
}

void ThreeBandEq::reset()
{
    bassFilter.reset();
    midFilter.reset();
    trebleFilter.reset();
}

void ThreeBandEq::setEnabled(bool shouldEnable)
{
    enabled = shouldEnable;
}

void ThreeBandEq::setBassGainDb(float gainDb)
{
    bassGainDb = gainDb;
}

void ThreeBandEq::setMidGainDb(float gainDb)
{
    midGainDb = gainDb;
}

void ThreeBandEq::setTrebleGainDb(float gainDb)
{
    trebleGainDb = gainDb;
}

void ThreeBandEq::process(juce::AudioBuffer<float>& buffer)
{
    if (!enabled || !prepared)
        return;

    updateFiltersIfNeeded();

    juce::dsp::AudioBlock<float> audioBlock(buffer);
    juce::dsp::ProcessContextReplacing<float> context(audioBlock);
    bassFilter.process(context);
    midFilter.process(context);
    trebleFilter.process(context);
}

void ThreeBandEq::updateFiltersIfNeeded()
{
    if (!prepared)
        return;

    const bool bassChanged = std::abs(bassGainDb - appliedBassGainDb) > gainChangeEpsilonDb;
    const bool midChanged = std::abs(midGainDb - appliedMidGainDb) > gainChangeEpsilonDb;
    const bool trebleChanged = std::abs(trebleGainDb - appliedTrebleGainDb) > gainChangeEpsilonDb;

    if (!bassChanged && !midChanged && !trebleChanged)
        return;

    if (bassChanged)
    {
        *bassFilter.state = *Coefficients::makeLowShelf(
            currentSpec.sampleRate,
            bassCenterFrequencyHertz,
            bassQualityFactor,
            juce::Decibels::decibelsToGain(bassGainDb));
        appliedBassGainDb = bassGainDb;
    }

    if (midChanged)
    {
        *midFilter.state = *Coefficients::makePeakFilter(
            currentSpec.sampleRate,
            midCenterFrequencyHertz,
            midQualityFactor,
            juce::Decibels::decibelsToGain(midGainDb));
        appliedMidGainDb = midGainDb;
    }

    if (trebleChanged)
    {
        *trebleFilter.state = *Coefficients::makeHighShelf(
            currentSpec.sampleRate,
            trebleCenterFrequencyHertz,
            trebleQualityFactor,
            juce::Decibels::decibelsToGain(trebleGainDb));
        appliedTrebleGainDb = trebleGainDb;
    }
}

} // namespace lumen::audio
