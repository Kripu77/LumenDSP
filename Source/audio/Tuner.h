#pragma once

#include <JuceHeader.h>
#include <atomic>
#include <vector>

namespace lumen::audio
{

class Tuner
{
public:
    void prepare(double sampleRate);
    void reset();
    void process(const juce::AudioBuffer<float>& buffer, int channelIndex = 0);

    float getFrequencyHz() const noexcept;
    float getCentsOffset() const noexcept;
    bool isLocked() const noexcept;
    juce::String getNoteName() const;

private:
    static constexpr int analysisSize = 2048;
    static constexpr int hopSize = 512;
    static constexpr float minFrequencyHz = 70.0f;
    static constexpr float maxFrequencyHz = 1200.0f;
    static constexpr float silenceThreshold = 0.004f;
    static constexpr float lockThreshold = 0.35f;

    void analyse();
    static juce::String midiNoteToName(int midiNote);

    double currentSampleRate = 48000.0;
    std::vector<float> ring;
    int writeIndex = 0;
    int samplesSinceAnalyse = 0;

    std::atomic<float> frequencyHz{0.0f};
    std::atomic<float> centsOffset{0.0f};
    std::atomic<bool> locked{false};
};

} // namespace lumen::audio
