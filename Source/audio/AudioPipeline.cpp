#include "audio/AudioPipeline.h"

namespace lumen::audio
{

void AudioPipeline::prepare(const juce::dsp::ProcessSpec& processSpec)
{
    currentSampleRate = processSpec.sampleRate > 0.0 ? processSpec.sampleRate : 44100.0;
    publishedSampleRate.store(currentSampleRate, std::memory_order_relaxed);
    currentBlockSize = juce::jmax(1, static_cast<int>(processSpec.maximumBlockSize));

    namEngine.prepare(processSpec.sampleRate, currentBlockSize);
    noiseGate.prepare(processSpec);
    compressor.prepare(processSpec);
    drive.prepare(processSpec);
    threeBandEq.prepare(processSpec);
    irConvolver.prepare(processSpec);
    doubler.prepare(processSpec);
    delay.prepare(processSpec);
    reverb.prepare(processSpec);
    inputMeter.prepare(processSpec.sampleRate);
    outputMeter.prepare(processSpec.sampleRate);
    tuner.prepare(processSpec.sampleRate);
    metronome.prepare(processSpec.sampleRate);
    demoPlayer.prepare(processSpec.sampleRate, currentBlockSize);
    onsetDetector.prepare(currentSampleRate);
    prepared = true;
    reset();
}

void AudioPipeline::reset()
{
    namEngine.reset();
    noiseGate.reset();
    compressor.reset();
    drive.reset();
    threeBandEq.reset();
    irConvolver.reset();
    doubler.reset();
    delay.reset();
    reverb.reset();
    inputMeter.reset();
    outputMeter.reset();
    tuner.reset();
    metronome.reset();
    demoPlayer.reset();
    onsetDetector.reset();
    // samplesProcessed deliberately keeps counting across resets: a device or buffer
    // change mid-take must not restart the clock that every practice event is stamped on.
    cpuLoadSmoothed = 0.0f;
    cpuLoadPercent.store(0.0f, std::memory_order_relaxed);
    hotInput.store(false, std::memory_order_relaxed);
    hotOutput.store(false, std::memory_order_relaxed);
}

void AudioPipeline::setControlState(const PipelineControlState& controlState)
{
    controls = controlState;

    noiseGate.setEnabled(controls.noiseGateEnabled);
    noiseGate.setThresholdDb(controls.noiseGateThresholdDb);

    compressor.setEnabled(controls.compressorEnabled);
    compressor.setThresholdDb(controls.compressorThresholdDb);
    compressor.setRatio(controls.compressorRatio);
    compressor.setMix(controls.compressorMix);

    drive.setEnabled(controls.driveEnabled);
    drive.setMode(static_cast<DriveMode>(juce::jlimit(0, 3, controls.driveMode)));
    drive.setDrive(controls.driveAmount);
    drive.setTone(controls.driveTone);
    drive.setLevel(controls.driveLevel);

    namEngine.setMix(controls.ampMix);

    threeBandEq.setEnabled(controls.eqEnabled);
    threeBandEq.setBassGainDb(controls.bassGainDb);
    threeBandEq.setMidGainDb(controls.midGainDb);
    threeBandEq.setTrebleGainDb(controls.trebleGainDb);

    irConvolver.setEnabled(controls.cabEnabled);
    irConvolver.setMix(controls.cabMix);

    delay.setEnabled(controls.delayEnabled);
    delay.setSyncEnabled(controls.delaySync);
    delay.setDivision(static_cast<DelayDivision>(juce::jlimit(0, 5, controls.delayDivision)));
    delay.setTempoBpm(controls.hostBpm > 0.0f ? controls.hostBpm : controls.metronomeBpm);
    delay.setTimeMs(controls.delayTimeMs);
    delay.setFeedback(controls.delayFeedback);
    delay.setMix(controls.delayMix);
    delay.setTone(controls.delayTone);
    delay.setModulation(controls.delayModulation);
    delay.setSaturation(controls.delaySaturation);
    delay.setPingPong(controls.delayPingPong);

    doubler.setEnabled(controls.doublerEnabled);
    doubler.setSpreadMs(controls.doublerSpreadMs);

    reverb.setEnabled(controls.reverbEnabled);
    reverb.setCharacter(static_cast<ReverbCharacter>(juce::jlimit(0, 3, controls.reverbCharacter)));
    reverb.setDecaySeconds(controls.reverbDecaySec);
    reverb.setLowCutHz(controls.reverbLowCutHz);
    reverb.setHighCutHz(controls.reverbHighCutHz);
    reverb.setWidth(controls.reverbWidth);
    reverb.setMix(controls.reverbMix);

    metronome.setEnabled(controls.metronomeEnabled);
    metronome.setBpm(controls.metronomeBpm);
    metronome.setVolumeLinear(controls.metronomeVolume);
}

void AudioPipeline::process(juce::AudioBuffer<float>& buffer)
{
    if (!prepared)
        return;

    const auto start = std::chrono::steady_clock::now();

    demoPlayer.processReplaceInput(buffer);

    inputMeter.processBlock(buffer, monoMeterChannelIndex);
    applyInputGain(buffer, controls.calibrationGainDb + controls.inputGainDb);
    tuner.process(buffer, monoMeterChannelIndex);
    const int64_t blockStartSample = samplesProcessed;
    captureInputEvents(buffer, blockStartSample);

    noiseGate.process(buffer);
    compressor.process(buffer);
    drive.process(buffer);
    namEngine.process(buffer);
    threeBandEq.process(buffer);
    irConvolver.process(buffer);
    doubler.process(buffer);
    delay.process(buffer);
    reverb.process(buffer);

    applyOutputLevel(buffer, controls.outputLevelDb);
    metronome.process(buffer);
    captureMetronomeEvents(blockStartSample);
    samplesProcessed += buffer.getNumSamples();
    outputMeter.processBlock(buffer, monoMeterChannelIndex);

    const auto end = std::chrono::steady_clock::now();
    const auto elapsedUs =
        std::chrono::duration_cast<std::chrono::duration<double, std::micro>>(end - start).count();
    const double blockUs =
        (static_cast<double>(buffer.getNumSamples()) / juce::jmax(1.0, currentSampleRate)) * 1.0e6;
    const float instantCpu =
        blockUs > 0.0 ? static_cast<float>(juce::jlimit(0.0, 200.0, (elapsedUs / blockUs) * 100.0))
                      : 0.0f;
    cpuLoadSmoothed = cpuLoadSmoothed * 0.9f + instantCpu * 0.1f;
    cpuLoadPercent.store(cpuLoadSmoothed, std::memory_order_relaxed);

    hotInput.store(
        inputMeter.getPeakLevelDb() >= hotInputThresholdDb,
        std::memory_order_relaxed);
    hotOutput.store(
        outputMeter.getPeakLevelDb() >= hotOutputThresholdDb,
        std::memory_order_relaxed);
}

NamEngine& AudioPipeline::getNamEngine() noexcept { return namEngine; }
IrConvolver& AudioPipeline::getIrConvolver() noexcept { return irConvolver; }
LevelMeter& AudioPipeline::getInputMeter() noexcept { return inputMeter; }
LevelMeter& AudioPipeline::getOutputMeter() noexcept { return outputMeter; }
Tuner& AudioPipeline::getTuner() noexcept { return tuner; }
Metronome& AudioPipeline::getMetronome() noexcept { return metronome; }
DemoPlayer& AudioPipeline::getDemoPlayer() noexcept { return demoPlayer; }

const NamEngine& AudioPipeline::getNamEngine() const noexcept { return namEngine; }
const IrConvolver& AudioPipeline::getIrConvolver() const noexcept { return irConvolver; }
const LevelMeter& AudioPipeline::getInputMeter() const noexcept { return inputMeter; }
const LevelMeter& AudioPipeline::getOutputMeter() const noexcept { return outputMeter; }
const Tuner& AudioPipeline::getTuner() const noexcept { return tuner; }
const Metronome& AudioPipeline::getMetronome() const noexcept { return metronome; }
const DemoPlayer& AudioPipeline::getDemoPlayer() const noexcept { return demoPlayer; }

PipelineDiagnostics AudioPipeline::getDiagnostics() const noexcept
{
    PipelineDiagnostics diagnostics;
    diagnostics.cpuLoadPercent = cpuLoadPercent.load(std::memory_order_relaxed);
    diagnostics.hotInput = hotInput.load(std::memory_order_relaxed);
    diagnostics.hotOutput = hotOutput.load(std::memory_order_relaxed);
    return diagnostics;
}

void AudioPipeline::captureInputEvents(const juce::AudioBuffer<float>& buffer, int64_t blockStartSample)
{
    using lumen::practice::PracticeEvent;
    using lumen::practice::PracticeEventKind;

    if (buffer.getNumChannels() <= 0)
        return;

    // The detector always runs so its envelopes are settled the moment capture starts.
    const bool isCapturing = practiceCaptureEnabled.load(std::memory_order_relaxed);
    onsetDetector.process(
        buffer.getReadPointer(monoMeterChannelIndex),
        buffer.getNumSamples(),
        [&](int sampleOffset, float strength) {
            if (isCapturing)
                practiceEvents.push(PracticeEvent{
                    PracticeEventKind::onset, blockStartSample + sampleOffset, strength, 1.0f});
        });

    if (!isCapturing)
        return;

    // Stamp pitch frames at the centre of their analysis window, where the estimate applies.
    constexpr int halfWindow = Tuner::getAnalysisWindowSize() / 2;
    for (int index = 0; index < tuner.getFrameCountInLastBlock(); ++index)
    {
        const auto& frame = tuner.getFrameInLastBlock(index);
        practiceEvents.push(PracticeEvent{
            PracticeEventKind::pitch,
            // sampleOffset is the last sample written, so the window centre is halfWindow - 1 back.
            blockStartSample + frame.sampleOffset - halfWindow + 1,
            frame.frequencyHz,
            frame.confidence});
    }
}

void AudioPipeline::captureMetronomeEvents(int64_t blockStartSample)
{
    using lumen::practice::PracticeEvent;
    using lumen::practice::PracticeEventKind;

    if (!practiceCaptureEnabled.load(std::memory_order_relaxed))
        return;

    for (int index = 0; index < metronome.getTickCountInLastBlock(); ++index)
    {
        const auto& tick = metronome.getTickInLastBlock(index);
        practiceEvents.push(PracticeEvent{
            PracticeEventKind::beat,
            blockStartSample + tick.sampleOffset,
            static_cast<float>(tick.beatInBar),
            1.0f});
    }
}

void AudioPipeline::setPracticeCaptureEnabled(bool shouldCapture) noexcept
{
    practiceCaptureEnabled.store(shouldCapture, std::memory_order_relaxed);
}

bool AudioPipeline::isPracticeCaptureEnabled() const noexcept
{
    return practiceCaptureEnabled.load(std::memory_order_relaxed);
}

size_t AudioPipeline::popPracticeEvents(lumen::practice::PracticeEvent* destination, size_t maxEvents) noexcept
{
    return practiceEvents.pop(destination, maxEvents);
}

void AudioPipeline::clearPracticeEvents() noexcept
{
    practiceEvents.clear();
}

uint32_t AudioPipeline::getDroppedPracticeEventCount() const noexcept
{
    return practiceEvents.droppedCount();
}

double AudioPipeline::getSampleRate() const noexcept
{
    return publishedSampleRate.load(std::memory_order_relaxed);
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
