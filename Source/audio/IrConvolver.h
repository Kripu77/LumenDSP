#pragma once

#include <JuceHeader.h>
#include <atomic>
#include <mutex>

namespace lumen::audio
{

class IrConvolver
{
public:
    IrConvolver();
    ~IrConvolver();

    void prepare(const juce::dsp::ProcessSpec& processSpec);
    void reset();
    void setEnabled(bool shouldEnable);
    bool loadImpulseResponseAsync(const juce::File& impulseResponseFile);
    void clearImpulseResponse();
    void process(juce::AudioBuffer<float>& buffer);

    bool isLoaded() const noexcept;
    juce::String getLoadedFileName() const;
    juce::File getLoadedFile() const;

private:
    static constexpr size_t maximumImpulseResponseLengthSamples = 1024 * 1024;
    static constexpr float dryWetFullyWet = 1.0f;

    void applyPendingImpulseResponse();

    juce::dsp::Convolution convolution;
    juce::dsp::ProcessSpec currentSpec{};
    juce::AudioBuffer<float> monoScratchBuffer;

    mutable std::mutex loadMutex;
    juce::AudioBuffer<float> pendingImpulseResponse;
    double pendingSampleRateHertz = 0.0;
    juce::File pendingFile;
    juce::File activeFile;
    juce::String activeFileName;

    std::atomic<bool> pendingLoadReady{false};
    std::atomic<bool> loaded{false};
    std::atomic<bool> enabled{true};
    bool prepared = false;
};

} // namespace lumen::audio
