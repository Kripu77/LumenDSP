#pragma once

#include <JuceHeader.h>
#include "audio/IrConvolver.h"
#include "audio/LevelMeter.h"
#include "audio/NamEngine.h"
#include "audio/NoiseGate.h"
#include "audio/ThreeBandEq.h"

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

    const NamEngine& getNamEngine() const noexcept;
    const IrConvolver& getIrConvolver() const noexcept;
    const LevelMeter& getInputMeter() const noexcept;
    const LevelMeter& getOutputMeter() const noexcept;

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

    PipelineControlState controls{};
    bool prepared = false;
};

} // namespace lumen::audio
