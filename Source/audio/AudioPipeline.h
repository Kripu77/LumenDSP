#pragma once

#include <JuceHeader.h>
#include "audio/IrConvolver.h"
#include "audio/LevelMeter.h"
#include "audio/Metronome.h"
#include "audio/NamEngine.h"
#include "audio/NoiseGate.h"
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
    ThreeBandEq threeBandEq;
    IrConvolver irConvolver;
    LevelMeter inputMeter;
    LevelMeter outputMeter;
    Tuner tuner;
    Metronome metronome;

    PipelineControlState controls{};
    bool prepared = false;
};

} // namespace lumen::audio
