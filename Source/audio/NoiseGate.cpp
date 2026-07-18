#include "audio/NoiseGate.h"

namespace lumen::audio
{

void NoiseGate::prepare(const juce::dsp::ProcessSpec& processSpec)
{
    sampleRateHertz = processSpec.sampleRate > 0.0 ? processSpec.sampleRate : 44100.0;
    attackCoefficient = computeEnvelopeCoefficient(attackTimeSeconds);
    releaseCoefficient = computeEnvelopeCoefficient(releaseTimeSeconds);
    reset();
}

void NoiseGate::reset()
{
    envelopeLinear = 0.0f;
    gainLinear = gainOpenLinear;
}

void NoiseGate::setEnabled(bool shouldEnable)
{
    enabled = shouldEnable;
}

void NoiseGate::setThresholdDb(float thresholdDb)
{
    thresholdLinear = linearFromDb(thresholdDb);
}

void NoiseGate::process(juce::AudioBuffer<float>& buffer)
{
    if (!enabled)
        return;

    const int channelCount = buffer.getNumChannels();
    const int sampleCount = buffer.getNumSamples();

    if (channelCount <= 0 || sampleCount <= 0)
        return;

    for (int sampleIndex = 0; sampleIndex < sampleCount; ++sampleIndex)
    {
        float detectorLinear = 0.0f;

        for (int channelIndex = 0; channelIndex < channelCount; ++channelIndex)
            detectorLinear = juce::jmax(
                detectorLinear,
                std::abs(buffer.getSample(channelIndex, sampleIndex)));

        const float coefficient = detectorLinear > envelopeLinear
                                      ? attackCoefficient
                                      : releaseCoefficient;
        envelopeLinear += coefficient * (detectorLinear - envelopeLinear);

        const float targetGainLinear = envelopeLinear >= thresholdLinear
                                           ? gainOpenLinear
                                           : gainClosedLinear;
        const float gainCoefficient = targetGainLinear > gainLinear
                                          ? attackCoefficient
                                          : releaseCoefficient;
        gainLinear += gainCoefficient * (targetGainLinear - gainLinear);

        for (int channelIndex = 0; channelIndex < channelCount; ++channelIndex)
        {
            const float inputSample = buffer.getSample(channelIndex, sampleIndex);
            buffer.setSample(channelIndex, sampleIndex, inputSample * gainLinear);
        }
    }
}

float NoiseGate::computeEnvelopeCoefficient(float timeSeconds) const
{
    if (timeSeconds <= 0.0f)
        return 1.0f;

    return 1.0f - std::exp(-1.0f / (timeSeconds * static_cast<float>(sampleRateHertz)));
}

float NoiseGate::linearFromDb(float levelDb) const
{
    return std::pow(10.0f, levelDb / decibelsPerNeper);
}

} // namespace lumen::audio
