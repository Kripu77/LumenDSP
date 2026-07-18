#pragma once

#include <JuceHeader.h>
#include "audio/Compressor.h"
#include "audio/Delay.h"
#include "audio/Drive.h"
#include "audio/IrConvolver.h"
#include "audio/LevelMeter.h"
#include "audio/Metronome.h"
#include "audio/NamEngine.h"
#include "audio/NoiseGate.h"
#include "audio/ReverbFx.h"
#include "audio/ThreeBandEq.h"
#include "audio/Tuner.h"

namespace lumen::audio
{

struct PipelineControlState
{
    float inputGainDb = 0.0f;
    float outputLevelDb = 0.0f;
    float noiseGateThresholdDb = -80.0f;
    bool noiseGateEnabled = true;
    float bassGainDb = 0.0f;
    float midGainDb = 0.0f;
    float trebleGainDb = 0.0f;
    bool eqEnabled = true;
    bool cabEnabled = true;
    bool metronomeEnabled = false;
    float metronomeBpm = 120.0f;
    float metronomeVolume = 0.35f;

    bool compressorEnabled = false;
    float compressorThresholdDb = -18.0f;
    float compressorRatio = 4.0f;
    float compressorMix = 1.0f;

    bool driveEnabled = false;
    int driveMode = 0;
    float driveAmount = 0.35f;
    float driveTone = 0.5f;
    float driveLevel = 0.7f;

    bool delayEnabled = false;
    bool delaySync = false;
    int delayDivision = 0;
    float delayTimeMs = 380.0f;
    float delayFeedback = 0.25f;
    float delayMix = 0.2f;

    bool reverbEnabled = false;
    int reverbCharacter = 0;
    float reverbSize = 0.4f;
    float reverbDamping = 0.5f;
    float reverbMix = 0.18f;
};

class AudioPipeline
{
public:
    void prepare(const juce::dsp::ProcessSpec& processSpec);
    void reset();
    void setControlState(const PipelineControlState& controlState);
    void process(juce::AudioBuffer<float>& buffer);

    NamEngine& getNamEngine() noexcept;
    IrConvolver& getIrConvolver() noexcept;
    LevelMeter& getInputMeter() noexcept;
    LevelMeter& getOutputMeter() noexcept;
    Tuner& getTuner() noexcept;
    Metronome& getMetronome() noexcept;

    const NamEngine& getNamEngine() const noexcept;
    const IrConvolver& getIrConvolver() const noexcept;
    const LevelMeter& getInputMeter() const noexcept;
    const LevelMeter& getOutputMeter() const noexcept;
    const Tuner& getTuner() const noexcept;
    const Metronome& getMetronome() const noexcept;

private:
    static constexpr int monoMeterChannelIndex = 0;

    void applyInputGain(juce::AudioBuffer<float>& buffer, float gainDb);
    void applyOutputLevel(juce::AudioBuffer<float>& buffer, float levelDb);

    NamEngine namEngine;
    NoiseGate noiseGate;
    Compressor compressor;
    Drive drive;
    ThreeBandEq threeBandEq;
    IrConvolver irConvolver;
    Delay delay;
    ReverbFx reverb;
    LevelMeter inputMeter;
    LevelMeter outputMeter;
    Tuner tuner;
    Metronome metronome;

    PipelineControlState controls{};
    bool prepared = false;
};

} // namespace lumen::audio
