#include "audio/LevelMeter.h"

namespace lumen::audio
{

namespace
{

constexpr float silenceFloorLinear = 1.0e-7f;
constexpr float decibelsPerNeper = 20.0f;

float linearToDb(float linearLevel)
{
    const float clampedLevel = juce::jmax(linearLevel, silenceFloorLinear);
    return decibelsPerNeper * std::log10(clampedLevel);
}

} // namespace

void LevelMeter::prepare(double newSampleRateHertz)
{
    sampleRateHertz = newSampleRateHertz > 0.0 ? newSampleRateHertz : 44100.0;
    reset();
}

void LevelMeter::reset()
{
    peakLevelLinear.store(0.0f, std::memory_order_relaxed);
    peakHoldLevelLinear.store(0.0f, std::memory_order_relaxed);
    peakHoldSamplesRemaining = 0;
}

void LevelMeter::processBlock(const float* samples, int sampleCount)
{
    if (samples == nullptr || sampleCount <= 0)
        return;

    float blockPeakLinear = 0.0f;

    for (int sampleIndex = 0; sampleIndex < sampleCount; ++sampleIndex)
        blockPeakLinear = juce::jmax(blockPeakLinear, std::abs(samples[sampleIndex]));

    const float previousPeakLinear = peakLevelLinear.load(std::memory_order_relaxed);
    const float decayFactor = std::exp(
        -static_cast<float>(sampleCount)
        * design::meterDecayPerSecond
        / static_cast<float>(sampleRateHertz));
    const float decayedPeakLinear = previousPeakLinear * decayFactor;
    const float nextPeakLinear = juce::jmax(blockPeakLinear, decayedPeakLinear);
    peakLevelLinear.store(nextPeakLinear, std::memory_order_relaxed);

    updatePeakHold(nextPeakLinear, sampleCount);
}

void LevelMeter::processBlock(const juce::AudioBuffer<float>& buffer, int channelIndex)
{
    if (channelIndex < 0 || channelIndex >= buffer.getNumChannels())
        return;

    processBlock(buffer.getReadPointer(channelIndex), buffer.getNumSamples());
}

float LevelMeter::getPeakLevelLinear() const noexcept
{
    return peakLevelLinear.load(std::memory_order_relaxed);
}

float LevelMeter::getPeakLevelDb() const noexcept
{
    return linearToDb(getPeakLevelLinear());
}

float LevelMeter::getPeakHoldLevelDb() const noexcept
{
    return linearToDb(peakHoldLevelLinear.load(std::memory_order_relaxed));
}

void LevelMeter::updatePeakHold(float nextPeakLinear, int sampleCount)
{
    const float currentHoldLinear = peakHoldLevelLinear.load(std::memory_order_relaxed);

    if (nextPeakLinear >= currentHoldLinear)
    {
        peakHoldLevelLinear.store(nextPeakLinear, std::memory_order_relaxed);
        peakHoldSamplesRemaining = static_cast<int>(
            design::meterPeakHoldSeconds * sampleRateHertz);
        return;
    }

    peakHoldSamplesRemaining = juce::jmax(0, peakHoldSamplesRemaining - sampleCount);

    if (peakHoldSamplesRemaining == 0)
        peakHoldLevelLinear.store(nextPeakLinear, std::memory_order_relaxed);
}

} // namespace lumen::audio
