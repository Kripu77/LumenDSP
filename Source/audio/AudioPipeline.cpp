#include "audio/AudioPipeline.h"

namespace lumen::audio
{

void AudioPipeline::prepare(const juce::dsp::ProcessSpec& processSpec)
{
    namEngine.prepare(processSpec.sampleRate, static_cast<int>(processSpec.maximumBlockSize));
    noiseGate.prepare(processSpec);
    threeBandEq.prepare(processSpec);
    irConvolver.prepare(processSpec);
    inputMeter.prepare(processSpec.sampleRate);
    outputMeter.prepare(processSpec.sampleRate);
    tuner.prepare(processSpec.sampleRate);
    metronome.prepare(processSpec.sampleRate);
    prepared = true;
    reset();
}

void AudioPipeline::reset()
{
    namEngine.reset();
    noiseGate.reset();
    threeBandEq.reset();
    irConvolver.reset();
    inputMeter.reset();
    outputMeter.reset();
    tuner.reset();
    metronome.reset();
}

void AudioPipeline::setControlState(const PipelineControlState& controlState)
{
    controls = controlState;
    noiseGate.setEnabled(controls.noiseGateEnabled);
    noiseGate.setThresholdDb(controls.noiseGateThresholdDb);
    threeBandEq.setEnabled(controls.eqEnabled);
    threeBandEq.setBassGainDb(controls.bassGainDb);
    threeBandEq.setMidGainDb(controls.midGainDb);
    threeBandEq.setTrebleGainDb(controls.trebleGainDb);
    irConvolver.setEnabled(controls.cabEnabled);
    metronome.setEnabled(controls.metronomeEnabled);
    metronome.setBpm(controls.metronomeBpm);
    metronome.setVolumeLinear(controls.metronomeVolume);
}

void AudioPipeline::process(juce::AudioBuffer<float>& buffer)
{
    if (!prepared)
        return;

    inputMeter.processBlock(buffer, monoMeterChannelIndex);
    applyInputGain(buffer, controls.inputGainDb);
    tuner.process(buffer, monoMeterChannelIndex);

    noiseGate.process(buffer);
    namEngine.process(buffer);
    threeBandEq.process(buffer);
    irConvolver.process(buffer);
    applyOutputLevel(buffer, controls.outputLevelDb);
    metronome.process(buffer);

    outputMeter.processBlock(buffer, monoMeterChannelIndex);
}

NamEngine& AudioPipeline::getNamEngine() noexcept
{
    return namEngine;
}

IrConvolver& AudioPipeline::getIrConvolver() noexcept
{
    return irConvolver;
}

LevelMeter& AudioPipeline::getInputMeter() noexcept
{
    return inputMeter;
}

LevelMeter& AudioPipeline::getOutputMeter() noexcept
{
    return outputMeter;
}

Tuner& AudioPipeline::getTuner() noexcept
{
    return tuner;
}

Metronome& AudioPipeline::getMetronome() noexcept
{
    return metronome;
}

const NamEngine& AudioPipeline::getNamEngine() const noexcept
{
    return namEngine;
}

const IrConvolver& AudioPipeline::getIrConvolver() const noexcept
{
    return irConvolver;
}

const LevelMeter& AudioPipeline::getInputMeter() const noexcept
{
    return inputMeter;
}

const LevelMeter& AudioPipeline::getOutputMeter() const noexcept
{
    return outputMeter;
}

const Tuner& AudioPipeline::getTuner() const noexcept
{
    return tuner;
}

const Metronome& AudioPipeline::getMetronome() const noexcept
{
    return metronome;
}

void AudioPipeline::applyInputGain(juce::AudioBuffer<float>& buffer, float gainDb)
{
    buffer.applyGain(juce::Decibels::decibelsToGain(gainDb));
}

void AudioPipeline::applyOutputLevel(juce::AudioBuffer<float>& buffer, float levelDb)
{
    buffer.applyGain(juce::Decibels::decibelsToGain(levelDb));
}

} // namespace lumen::audio
