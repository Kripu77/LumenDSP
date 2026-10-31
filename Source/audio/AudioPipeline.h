#pragma once

#include <JuceHeader.h>
#include "audio/Compressor.h"
#include "audio/Delay.h"
#include "audio/DemoPlayer.h"
#include "audio/Doubler.h"
#include "audio/Drive.h"
#include "audio/IrConvolver.h"
#include "audio/LevelMeter.h"
#include "audio/Metronome.h"
#include "audio/NamEngine.h"
#include "audio/NoiseGate.h"
#include "audio/ReverbFx.h"
#include "audio/ThreeBandEq.h"
#include "audio/Tuner.h"
#include "practice/OnsetDetector.h"
#include "practice/PracticeEventQueue.h"
#include <atomic>
#include <chrono>

namespace lumen::audio
{

struct PipelineControlState
{
    float inputGainDb = 0.0f;
    float calibrationGainDb = 0.0f;
    float outputLevelDb = 0.0f;
    float noiseGateThresholdDb = -80.0f;
    bool noiseGateEnabled = true;
    float bassGainDb = 0.0f;
    float midGainDb = 0.0f;
    float trebleGainDb = 0.0f;
    bool eqEnabled = true;
    bool cabEnabled = true;
    float ampMix = 1.0f;
    float cabMix = 1.0f;
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
    float delayTone = 0.5f;
    float delayModulation = 0.3f;
    float delaySaturation = 0.2f;
    bool delayPingPong = false;
    bool doublerEnabled = false;
    float doublerSpreadMs = 6.0f;
    float hostBpm = 0.0f;

    bool reverbEnabled = false;
    int reverbCharacter = 0;
    float reverbDecaySec = 4.0f;
    float reverbLowCutHz = 180.0f;
    float reverbHighCutHz = 8000.0f;
    float reverbWidth = 1.0f;
    float reverbMix = 0.18f;
};

struct PipelineDiagnostics
{
    float cpuLoadPercent = 0.0f;
    bool hotInput = false;
    bool hotOutput = false;
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
    DemoPlayer& getDemoPlayer() noexcept;

    const NamEngine& getNamEngine() const noexcept;
    const IrConvolver& getIrConvolver() const noexcept;
    const LevelMeter& getInputMeter() const noexcept;
    const LevelMeter& getOutputMeter() const noexcept;
    const Tuner& getTuner() const noexcept;
    const Metronome& getMetronome() const noexcept;
    const DemoPlayer& getDemoPlayer() const noexcept;

    PipelineDiagnostics getDiagnostics() const noexcept;

    // Practice capture: when enabled, the audio thread streams metronome clicks, pick onsets
    // and pitch frames (all stamped on one sample clock) for the message thread to drain.
    static constexpr size_t practiceQueueCapacity = 4096;
    void setPracticeCaptureEnabled(bool shouldCapture) noexcept;
    bool isPracticeCaptureEnabled() const noexcept;
    size_t popPracticeEvents(lumen::practice::PracticeEvent* destination, size_t maxEvents) noexcept;
    // Message thread only. Drops anything queued so a new take starts clean.
    void clearPracticeEvents() noexcept;
    uint32_t getDroppedPracticeEventCount() const noexcept;
    double getSampleRate() const noexcept;

private:
    static constexpr int monoMeterChannelIndex = 0;
    static constexpr float hotInputThresholdDb = -3.0f;
    static constexpr float hotOutputThresholdDb = -1.0f;

    void applyInputGain(juce::AudioBuffer<float>& buffer, float gainDb);
    void applyOutputLevel(juce::AudioBuffer<float>& buffer, float levelDb);
    void captureInputEvents(const juce::AudioBuffer<float>& buffer, int64_t blockStartSample);
    void captureMetronomeEvents(int64_t blockStartSample);

    NamEngine namEngine;
    NoiseGate noiseGate;
    Compressor compressor;
    Drive drive;
    ThreeBandEq threeBandEq;
    IrConvolver irConvolver;
    Doubler doubler;
    Delay delay;
    ReverbFx reverb;
    LevelMeter inputMeter;
    LevelMeter outputMeter;
    Tuner tuner;
    Metronome metronome;
    DemoPlayer demoPlayer;
    lumen::practice::OnsetDetector onsetDetector;
    lumen::practice::PracticeEventQueue<practiceQueueCapacity> practiceEvents;
    std::atomic<bool> practiceCaptureEnabled{false};
    // Absolute sample clock shared by every practice event. 64-bit: never wraps in practice.
    int64_t samplesProcessed = 0;
    // Copy of currentSampleRate the message thread can read without a data race.
    std::atomic<double> publishedSampleRate{44100.0};

    PipelineControlState controls{};
    bool prepared = false;
    double currentSampleRate = 44100.0;
    int currentBlockSize = 512;

    std::atomic<float> cpuLoadPercent{0.0f};
    std::atomic<bool> hotInput{false};
    std::atomic<bool> hotOutput{false};
    float cpuLoadSmoothed = 0.0f;
};

} // namespace lumen::audio
