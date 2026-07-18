#pragma once

#include <JuceHeader.h>

namespace lumen::parameters
{

inline constexpr const char* inputGainId = "inputGain";
inline constexpr const char* outputLevelId = "outputLevel";
inline constexpr const char* noiseGateThresholdId = "noiseGateThreshold";
inline constexpr const char* noiseGateEnabledId = "noiseGateEnabled";
inline constexpr const char* bassGainId = "bassGain";
inline constexpr const char* midGainId = "midGain";
inline constexpr const char* trebleGainId = "trebleGain";
inline constexpr const char* eqEnabledId = "eqEnabled";
inline constexpr const char* cabEnabledId = "cabEnabled";
inline constexpr const char* metronomeEnabledId = "metronomeEnabled";
inline constexpr const char* metronomeBpmId = "metronomeBpm";
inline constexpr const char* metronomeVolumeId = "metronomeVolume";

inline constexpr const char* compressorEnabledId = "compressorEnabled";
inline constexpr const char* compressorThresholdId = "compressorThreshold";
inline constexpr const char* compressorRatioId = "compressorRatio";
inline constexpr const char* compressorMixId = "compressorMix";

inline constexpr const char* driveEnabledId = "driveEnabled";
inline constexpr const char* driveAmountId = "driveAmount";
inline constexpr const char* driveToneId = "driveTone";
inline constexpr const char* driveLevelId = "driveLevel";

inline constexpr const char* delayEnabledId = "delayEnabled";
inline constexpr const char* delayTimeId = "delayTime";
inline constexpr const char* delayFeedbackId = "delayFeedback";
inline constexpr const char* delayMixId = "delayMix";

inline constexpr const char* reverbEnabledId = "reverbEnabled";
inline constexpr const char* reverbSizeId = "reverbSize";
inline constexpr const char* reverbDampingId = "reverbDamping";
inline constexpr const char* reverbMixId = "reverbMix";

namespace ranges
{

inline constexpr float inputGainMinimumDb = -24.0f;
inline constexpr float inputGainMaximumDb = 24.0f;
inline constexpr float inputGainDefaultDb = 0.0f;

inline constexpr float outputLevelMinimumDb = -40.0f;
inline constexpr float outputLevelMaximumDb = 12.0f;
inline constexpr float outputLevelDefaultDb = 0.0f;

inline constexpr float noiseGateThresholdMinimumDb = -100.0f;
inline constexpr float noiseGateThresholdMaximumDb = 0.0f;
inline constexpr float noiseGateThresholdDefaultDb = -80.0f;

inline constexpr float eqGainMinimumDb = -12.0f;
inline constexpr float eqGainMaximumDb = 12.0f;
inline constexpr float eqGainDefaultDb = 0.0f;

inline constexpr bool noiseGateEnabledDefault = true;
inline constexpr bool eqEnabledDefault = true;
inline constexpr bool cabEnabledDefault = true;
inline constexpr bool metronomeEnabledDefault = false;
inline constexpr bool compressorEnabledDefault = false;
inline constexpr bool driveEnabledDefault = false;
inline constexpr bool delayEnabledDefault = false;
inline constexpr bool reverbEnabledDefault = false;

inline constexpr float metronomeBpmMinimum = 40.0f;
inline constexpr float metronomeBpmMaximum = 240.0f;
inline constexpr float metronomeBpmDefault = 120.0f;

inline constexpr float unitMinimum = 0.0f;
inline constexpr float unitMaximum = 1.0f;
inline constexpr float metronomeVolumeDefault = 0.35f;

inline constexpr float compressorThresholdMinimumDb = -40.0f;
inline constexpr float compressorThresholdMaximumDb = 0.0f;
inline constexpr float compressorThresholdDefaultDb = -18.0f;
inline constexpr float compressorRatioMinimum = 1.0f;
inline constexpr float compressorRatioMaximum = 20.0f;
inline constexpr float compressorRatioDefault = 4.0f;
inline constexpr float compressorMixDefault = 1.0f;

inline constexpr float driveAmountDefault = 0.35f;
inline constexpr float driveToneDefault = 0.5f;
inline constexpr float driveLevelDefault = 0.7f;

inline constexpr float delayTimeMinimumMs = 1.0f;
inline constexpr float delayTimeMaximumMs = 1000.0f;
inline constexpr float delayTimeDefaultMs = 380.0f;
inline constexpr float delayFeedbackDefault = 0.25f;
inline constexpr float delayMixDefault = 0.2f;

inline constexpr float reverbSizeDefault = 0.4f;
inline constexpr float reverbDampingDefault = 0.5f;
inline constexpr float reverbMixDefault = 0.18f;

} // namespace ranges

namespace detail
{

inline void addFloat(
    std::vector<std::unique_ptr<juce::RangedAudioParameter>>& parameters,
    const char* id,
    const juce::String& name,
    float minValue,
    float maxValue,
    float defaultValue,
    float step,
    const juce::String& label = {})
{
    auto attributes = juce::AudioParameterFloatAttributes();
    if (label.isNotEmpty())
        attributes = attributes.withLabel(label);

    parameters.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{id, 1},
        name,
        juce::NormalisableRange<float>(minValue, maxValue, step),
        defaultValue,
        attributes));
}

inline void addBool(
    std::vector<std::unique_ptr<juce::RangedAudioParameter>>& parameters,
    const char* id,
    const juce::String& name,
    bool defaultValue)
{
    parameters.push_back(std::make_unique<juce::AudioParameterBool>(
        juce::ParameterID{id, 1},
        name,
        defaultValue));
}

} // namespace detail

inline juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> parameters;

    detail::addFloat(parameters, inputGainId, "Input", ranges::inputGainMinimumDb, ranges::inputGainMaximumDb, ranges::inputGainDefaultDb, 0.1f, "dB");
    detail::addFloat(parameters, noiseGateThresholdId, "Gate", ranges::noiseGateThresholdMinimumDb, ranges::noiseGateThresholdMaximumDb, ranges::noiseGateThresholdDefaultDb, 0.1f, "dB");
    detail::addBool(parameters, noiseGateEnabledId, "Gate On", ranges::noiseGateEnabledDefault);

    detail::addBool(parameters, compressorEnabledId, "Comp On", ranges::compressorEnabledDefault);
    detail::addFloat(parameters, compressorThresholdId, "Comp Thresh", ranges::compressorThresholdMinimumDb, ranges::compressorThresholdMaximumDb, ranges::compressorThresholdDefaultDb, 0.1f, "dB");
    detail::addFloat(parameters, compressorRatioId, "Comp Ratio", ranges::compressorRatioMinimum, ranges::compressorRatioMaximum, ranges::compressorRatioDefault, 0.1f);
    detail::addFloat(parameters, compressorMixId, "Comp Mix", ranges::unitMinimum, ranges::unitMaximum, ranges::compressorMixDefault, 0.01f);

    detail::addBool(parameters, driveEnabledId, "Drive On", ranges::driveEnabledDefault);
    detail::addFloat(parameters, driveAmountId, "Drive", ranges::unitMinimum, ranges::unitMaximum, ranges::driveAmountDefault, 0.01f);
    detail::addFloat(parameters, driveToneId, "Drive Tone", ranges::unitMinimum, ranges::unitMaximum, ranges::driveToneDefault, 0.01f);
    detail::addFloat(parameters, driveLevelId, "Drive Level", ranges::unitMinimum, ranges::unitMaximum, ranges::driveLevelDefault, 0.01f);

    detail::addFloat(parameters, bassGainId, "Bass", ranges::eqGainMinimumDb, ranges::eqGainMaximumDb, ranges::eqGainDefaultDb, 0.1f, "dB");
    detail::addFloat(parameters, midGainId, "Mid", ranges::eqGainMinimumDb, ranges::eqGainMaximumDb, ranges::eqGainDefaultDb, 0.1f, "dB");
    detail::addFloat(parameters, trebleGainId, "Treble", ranges::eqGainMinimumDb, ranges::eqGainMaximumDb, ranges::eqGainDefaultDb, 0.1f, "dB");
    detail::addBool(parameters, eqEnabledId, "EQ On", ranges::eqEnabledDefault);
    detail::addBool(parameters, cabEnabledId, "Cab On", ranges::cabEnabledDefault);

    detail::addBool(parameters, delayEnabledId, "Delay On", ranges::delayEnabledDefault);
    detail::addFloat(parameters, delayTimeId, "Delay Time", ranges::delayTimeMinimumMs, ranges::delayTimeMaximumMs, ranges::delayTimeDefaultMs, 1.0f, "ms");
    detail::addFloat(parameters, delayFeedbackId, "Delay Fdbk", ranges::unitMinimum, ranges::unitMaximum, ranges::delayFeedbackDefault, 0.01f);
    detail::addFloat(parameters, delayMixId, "Delay Mix", ranges::unitMinimum, ranges::unitMaximum, ranges::delayMixDefault, 0.01f);

    detail::addBool(parameters, reverbEnabledId, "Reverb On", ranges::reverbEnabledDefault);
    detail::addFloat(parameters, reverbSizeId, "Reverb Size", ranges::unitMinimum, ranges::unitMaximum, ranges::reverbSizeDefault, 0.01f);
    detail::addFloat(parameters, reverbDampingId, "Reverb Damp", ranges::unitMinimum, ranges::unitMaximum, ranges::reverbDampingDefault, 0.01f);
    detail::addFloat(parameters, reverbMixId, "Reverb Mix", ranges::unitMinimum, ranges::unitMaximum, ranges::reverbMixDefault, 0.01f);

    detail::addFloat(parameters, outputLevelId, "Output", ranges::outputLevelMinimumDb, ranges::outputLevelMaximumDb, ranges::outputLevelDefaultDb, 0.1f, "dB");
    detail::addBool(parameters, metronomeEnabledId, "Metronome", ranges::metronomeEnabledDefault);
    detail::addFloat(parameters, metronomeBpmId, "Tempo", ranges::metronomeBpmMinimum, ranges::metronomeBpmMaximum, ranges::metronomeBpmDefault, 1.0f, "BPM");
    detail::addFloat(parameters, metronomeVolumeId, "Click Level", ranges::unitMinimum, ranges::unitMaximum, ranges::metronomeVolumeDefault, 0.01f);

    return {parameters.begin(), parameters.end()};
}

} // namespace lumen::parameters
