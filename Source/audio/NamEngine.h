#pragma once

#include <JuceHeader.h>
#include <NAM/dsp.h>
#include <NAM/get_dsp.h>
#include <atomic>
#include <memory>
#include <mutex>
#include <vector>

namespace lumen::audio
{

class NamEngine
{
public:
    NamEngine();
    ~NamEngine();

    void prepare(double sampleRateHertz, int maximumBlockSizeSamples);
    void reset();
    bool loadModelAsync(const juce::File& modelFile);
    void clearModel();
    void process(juce::AudioBuffer<float>& buffer);

    bool isModelLoaded() const noexcept;
    bool isModelLoading() const noexcept;
    juce::String getLoadedModelName() const;
    juce::File getLoadedModelFile() const;
    juce::String getLastErrorMessage() const;

private:
    static constexpr int monoChannelCount = 1;
    static constexpr int monoChannelIndex = 0;

    using ModelPtr = std::shared_ptr<nam::DSP>;

    void promoteStagedModelLocked();
    void processMonoThroughModel(
        nam::DSP& model,
        float* channelData,
        int sampleCount);

    double currentSampleRateHertz = 44100.0;
    int currentMaximumBlockSizeSamples = 0;

    // shared_ptr so the audio thread can keep a model alive for one block
    // even if a background load swaps `activeModel` mid-callback.
    ModelPtr activeModel;
    ModelPtr stagedModel;

    std::vector<NAM_SAMPLE> inputScratch;
    std::vector<NAM_SAMPLE> outputScratch;
    NAM_SAMPLE* inputPointers[monoChannelCount]{};
    NAM_SAMPLE* outputPointers[monoChannelCount]{};

    mutable std::mutex modelMutex;
    juce::File stagedModelFile;
    juce::String stagedModelName;
    juce::File activeModelFile;
    juce::String activeModelName;
    juce::String lastErrorMessage;

    std::atomic<bool> stagedModelReady{false};
    std::atomic<bool> modelLoaded{false};
    std::atomic<bool> modelLoading{false};
    std::atomic<bool> prepared{false};
};

} // namespace lumen::audio
