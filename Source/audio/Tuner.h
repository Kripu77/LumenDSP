#pragma once

#include <JuceHeader.h>
#include <array>
#include <atomic>
#include <vector>

namespace lumen::audio
{

// One pitch estimate. sampleOffset is where the analysis window ENDED inside the block;
// subtract getAnalysisWindowSize() / 2 to get the window centre.
struct TunerFrame
{
    int sampleOffset = 0;
    // 0 when the window was silent or the estimate was not confident enough to lock.
    float frequencyHz = 0.0f;
    float confidence = 0.0f;
};

class Tuner
{
public:
    static constexpr int analysisSize = 2048;
    static constexpr int hopSize = 512;
    // Enough for an 8192-sample block (8192 / 512 = 16).
    static constexpr int maxFramesPerBlock = 16;

    void prepare(double sampleRate);
    void reset();
    void process(const juce::AudioBuffer<float>& buffer, int channelIndex = 0);

    float getFrequencyHz() const noexcept;
    float getCentsOffset() const noexcept;
    bool isLocked() const noexcept;
    juce::String getNoteName() const;

    // Audio thread only: every analysis frame produced by the last process() call.
    // The vibrato analyser needs the full frame stream, not the 30 Hz UI snapshot.
    int getFrameCountInLastBlock() const noexcept;
    const TunerFrame& getFrameInLastBlock(int index) const noexcept;
    static constexpr int getAnalysisWindowSize() noexcept { return analysisSize; }

private:
    static constexpr float minFrequencyHz = 70.0f;
    static constexpr float maxFrequencyHz = 1200.0f;
    static constexpr float silenceThreshold = 0.004f;
    static constexpr float lockThreshold = 0.35f;

    // Returns the frequency it locked to, or 0 when it could not lock.
    TunerFrame analyse();
    float autocorrelationAt(int lag) const noexcept;
    void copyRingInTimeOrder() noexcept;
    void storeUnlocked() noexcept;
    static juce::String midiNoteToName(int midiNote);

    double currentSampleRate = 48000.0;
    std::array<TunerFrame, maxFramesPerBlock> framesInLastBlock{};
    int frameCountInLastBlock = 0;
    std::vector<float> ring;
    // Scratch copy of the ring in time order; allocated in prepare(), reused every hop.
    std::vector<float> window;
    int writeIndex = 0;
    int samplesSinceAnalyse = 0;

    std::atomic<float> frequencyHz{0.0f};
    std::atomic<float> centsOffset{0.0f};
    std::atomic<bool> locked{false};
};

} // namespace lumen::audio
