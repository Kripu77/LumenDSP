#include "audio/Metronome.h"
#include <cmath>
#include <atomic>

namespace lumen::audio
{

void Metronome::prepare(double sampleRate)
{
    currentSampleRate = sampleRate > 0.0 ? sampleRate : 48000.0;
    setBpm(120.0f);
    reset();
}

void Metronome::reset()
{
    sampleCounter = 0.0;
    beatInBar.store(0, std::memory_order_relaxed);
    clickSamplesRemaining = 0;
    clickPhase = 0.0f;
}

void Metronome::setEnabled(bool shouldEnable) noexcept
{
    enabled = shouldEnable;
    if (!enabled)
    {
        clickSamplesRemaining = 0;
        sampleCounter = 0.0;
        beatInBar.store(0, std::memory_order_relaxed);
    }
}

int Metronome::getBeatInBar() const noexcept
{
    return beatInBar.load(std::memory_order_relaxed);
}

int Metronome::getBeatsPerBar() const noexcept
{
    return beatsPerBar;
}

void Metronome::setBpm(float bpm) noexcept
{
    const float clamped = juce::jlimit(minimumBpm, maximumBpm, bpm);
    samplesPerBeat = currentSampleRate * 60.0 / static_cast<double>(clamped);
    if (samplesPerBeat < 1.0)
        samplesPerBeat = 1.0;
}

void Metronome::setVolumeLinear(float volume01) noexcept
{
    volumeLinear = juce::jlimit(0.0f, 1.0f, volume01);
}

void Metronome::process(juce::AudioBuffer<float>& buffer)
{
    if (!enabled || buffer.getNumSamples() <= 0)
        return;

    const int numSamples = buffer.getNumSamples();
    const int numChannels = buffer.getNumChannels();
    const int clickLength = juce::jmax(1, static_cast<int>(currentSampleRate * clickDurationSeconds));

    for (int i = 0; i < numSamples; ++i)
    {
        if (sampleCounter >= samplesPerBeat)
        {
            sampleCounter -= samplesPerBeat;
            const int beat = beatInBar.load(std::memory_order_relaxed);
            const bool accent = (beat == 0);
            const float freq = accent ? accentFrequencyHz : clickFrequencyHz;
            clickPhaseDelta = static_cast<float>(juce::MathConstants<double>::twoPi * freq / currentSampleRate);
            clickPhase = 0.0f;
            clickAmplitude = volumeLinear * (accent ? 0.85f : 0.55f);
            clickSamplesRemaining = clickLength;
            beatInBar.store((beat + 1) % beatsPerBar, std::memory_order_relaxed);
        }

        float click = 0.0f;
        if (clickSamplesRemaining > 0)
        {
            const float env = static_cast<float>(clickSamplesRemaining) / static_cast<float>(clickLength);
            click = std::sin(clickPhase) * clickAmplitude * env * env;
            clickPhase += clickPhaseDelta;
            --clickSamplesRemaining;
        }

        for (int ch = 0; ch < numChannels; ++ch)
            buffer.addSample(ch, i, click);

        sampleCounter += 1.0;
    }
}

} // namespace lumen::audio
