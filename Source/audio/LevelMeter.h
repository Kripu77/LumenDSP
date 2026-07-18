#pragma once

#include <JuceHeader.h>
#include "ui/DesignTokens.h"

namespace lumen::audio
{

class LevelMeter
{
public:
    void prepare(double sampleRateHertz);
    void reset();
    void processBlock(const float* samples, int sampleCount);
    void processBlock(const juce::AudioBuffer<float>& buffer, int channelIndex);

    float getPeakLevelLinear() const noexcept;
    float getPeakLevelDb() const noexcept;
    float getPeakHoldLevelDb() const noexcept;

private:
    void updatePeakHold(float peakLevelLinear, int sampleCount);

    double sampleRateHertz = 44100.0;
    std::atomic<float> peakLevelLinear{0.0f};
    std::atomic<float> peakHoldLevelLinear{0.0f};
    int peakHoldSamplesRemaining = 0;
};

} // namespace lumen::audio
