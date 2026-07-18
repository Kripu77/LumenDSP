#pragma once

#include <JuceHeader.h>
#include <atomic>

namespace lumen::audio
{

class Metronome
{
public:
    void prepare(double sampleRate);
    void reset();

    void setEnabled(bool shouldEnable) noexcept;
    void setBpm(float bpm) noexcept;
    void setVolumeLinear(float volume01) noexcept;

    void process(juce::AudioBuffer<float>& buffer);

    int getBeatInBar() const noexcept;
    int getBeatsPerBar() const noexcept;

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

    bool enabled = false;
    float volumeLinear = 0.35f;
};

} // namespace lumen::audio
