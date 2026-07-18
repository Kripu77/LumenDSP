#pragma once

#include <JuceHeader.h>

namespace lumen::audio
{

class NoiseGate
{
public:
    void prepare(const juce::dsp::ProcessSpec& processSpec);
    void reset();
    void setEnabled(bool shouldEnable);
    void setThresholdDb(float thresholdDb);
    void process(juce::AudioBuffer<float>& buffer);

private:
    static constexpr float attackTimeSeconds = 0.001f;
    static constexpr float releaseTimeSeconds = 0.050f;
    static constexpr float envelopeFloorLinear = 1.0e-7f;
    static constexpr float gainOpenLinear = 1.0f;
    static constexpr float gainClosedLinear = 0.0f;
    static constexpr float decibelsPerNeper = 20.0f;

    float computeEnvelopeCoefficient(float timeSeconds) const;
    float linearFromDb(float levelDb) const;

    double sampleRateHertz = 44100.0;
    bool enabled = true;
    float thresholdLinear = 0.0001f;
    float envelopeLinear = 0.0f;
    float gainLinear = 1.0f;
    float attackCoefficient = 0.0f;
    float releaseCoefficient = 0.0f;
};

} // namespace lumen::audio
