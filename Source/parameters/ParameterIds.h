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

namespace ranges
{

inline constexpr float inputGainMinimumDb = -24.0f;
inline constexpr float inputGainMaximumDb = 24.0f;
inline constexpr float inputGainDefaultDb = 0.0f;
inline constexpr float inputGainStepDb = 0.1f;

inline constexpr float outputLevelMinimumDb = -40.0f;
inline constexpr float outputLevelMaximumDb = 12.0f;
inline constexpr float outputLevelDefaultDb = 0.0f;
inline constexpr float outputLevelStepDb = 0.1f;

inline constexpr float noiseGateThresholdMinimumDb = -100.0f;
inline constexpr float noiseGateThresholdMaximumDb = 0.0f;
inline constexpr float noiseGateThresholdDefaultDb = -80.0f;
inline constexpr float noiseGateThresholdStepDb = 0.1f;

inline constexpr float eqGainMinimumDb = -12.0f;
inline constexpr float eqGainMaximumDb = 12.0f;
inline constexpr float eqGainDefaultDb = 0.0f;
inline constexpr float eqGainStepDb = 0.1f;

inline constexpr bool noiseGateEnabledDefault = true;
inline constexpr bool eqEnabledDefault = true;
inline constexpr bool cabEnabledDefault = true;

} // namespace ranges

inline juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> parameters;

    parameters.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{inputGainId, 1},
        "Input",
        juce::NormalisableRange<float>(
            ranges::inputGainMinimumDb,
            ranges::inputGainMaximumDb,
            ranges::inputGainStepDb),
        ranges::inputGainDefaultDb,
        juce::AudioParameterFloatAttributes().withLabel("dB")));

    parameters.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{noiseGateThresholdId, 1},
        "Gate",
        juce::NormalisableRange<float>(
            ranges::noiseGateThresholdMinimumDb,
            ranges::noiseGateThresholdMaximumDb,
            ranges::noiseGateThresholdStepDb),
        ranges::noiseGateThresholdDefaultDb,
        juce::AudioParameterFloatAttributes().withLabel("dB")));

    parameters.push_back(std::make_unique<juce::AudioParameterBool>(
        juce::ParameterID{noiseGateEnabledId, 1},
        "Gate On",
        ranges::noiseGateEnabledDefault));

    parameters.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{bassGainId, 1},
        "Bass",
        juce::NormalisableRange<float>(
            ranges::eqGainMinimumDb,
            ranges::eqGainMaximumDb,
            ranges::eqGainStepDb),
        ranges::eqGainDefaultDb,
        juce::AudioParameterFloatAttributes().withLabel("dB")));

    parameters.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{midGainId, 1},
        "Mid",
        juce::NormalisableRange<float>(
            ranges::eqGainMinimumDb,
            ranges::eqGainMaximumDb,
            ranges::eqGainStepDb),
        ranges::eqGainDefaultDb,
        juce::AudioParameterFloatAttributes().withLabel("dB")));

    parameters.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{trebleGainId, 1},
        "Treble",
        juce::NormalisableRange<float>(
            ranges::eqGainMinimumDb,
            ranges::eqGainMaximumDb,
            ranges::eqGainStepDb),
        ranges::eqGainDefaultDb,
        juce::AudioParameterFloatAttributes().withLabel("dB")));

    parameters.push_back(std::make_unique<juce::AudioParameterBool>(
        juce::ParameterID{eqEnabledId, 1},
        "EQ On",
        ranges::eqEnabledDefault));

    parameters.push_back(std::make_unique<juce::AudioParameterBool>(
        juce::ParameterID{cabEnabledId, 1},
        "Cab On",
        ranges::cabEnabledDefault));

    parameters.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{outputLevelId, 1},
        "Output",
        juce::NormalisableRange<float>(
            ranges::outputLevelMinimumDb,
            ranges::outputLevelMaximumDb,
            ranges::outputLevelStepDb),
        ranges::outputLevelDefaultDb,
        juce::AudioParameterFloatAttributes().withLabel("dB")));

    return {parameters.begin(), parameters.end()};
}

} // namespace lumen::parameters
