#pragma once

#include <JuceHeader.h>
#include <array>
#include <atomic>

namespace lumen::audio
{

// Where a click started inside the most recent process() block.
struct MetronomeTick
{
    int sampleOffset = 0;
    int beatInBar = 0;
};

class Metronome
{
public:
    // Comfortably above the clicks one block can hold (240 BPM, 8192-sample block at 44.1 kHz is ~1).
    static constexpr int maxTicksPerBlock = 8;

    void prepare(double sampleRate);
    void reset();

    void setEnabled(bool shouldEnable) noexcept;
    void setBpm(float bpm) noexcept;
    void setVolumeLinear(float volume01) noexcept;

    void process(juce::AudioBuffer<float>& buffer);

    int getBeatInBar() const noexcept;
    int getBeatsPerBar() const noexcept;

    // Audio thread only: the clicks started during the last process() call, so practice
    // capture can timestamp them on the same sample clock as the guitar input.
    int getTickCountInLastBlock() const noexcept;
    const MetronomeTick& getTickInLastBlock(int index) const noexcept;

private:
    static constexpr float minimumBpm = 40.0f;
    static constexpr float maximumBpm = 240.0f;
    static constexpr float clickFrequencyHz = 1000.0f;
    static constexpr float accentFrequencyHz = 1400.0f;
    static constexpr float clickDurationSeconds = 0.018f;

    double currentSampleRate = 48000.0;
    double samplesPerBeat = 24000.0;
    double sampleCounter = 0.0;
    std::atomic<int> beatInBar{0};
    int beatsPerBar = 4;

    int clickSamplesRemaining = 0;
    float clickPhase = 0.0f;
    float clickPhaseDelta = 0.0f;
    float clickAmplitude = 0.0f;

    std::array<MetronomeTick, maxTicksPerBlock> ticksInLastBlock{};
    int tickCountInLastBlock = 0;

    bool enabled = false;
    float volumeLinear = 0.35f;
};

} // namespace lumen::audio
