#include "audio/Tuner.h"
#include <cmath>

namespace lumen::audio
{

void Tuner::prepare(double sampleRate)
{
    currentSampleRate = sampleRate > 0.0 ? sampleRate : 48000.0;
    ring.assign(static_cast<size_t>(analysisSize), 0.0f);
    writeIndex = 0;
    samplesSinceAnalyse = 0;
    reset();
}

void Tuner::reset()
{
    frequencyHz.store(0.0f, std::memory_order_relaxed);
    centsOffset.store(0.0f, std::memory_order_relaxed);
    locked.store(false, std::memory_order_relaxed);
    std::fill(ring.begin(), ring.end(), 0.0f);
    writeIndex = 0;
    samplesSinceAnalyse = 0;
}

void Tuner::process(const juce::AudioBuffer<float>& buffer, int channelIndex)
{
    if (ring.empty() || buffer.getNumChannels() <= 0)
        return;

    const int channel = juce::jlimit(0, buffer.getNumChannels() - 1, channelIndex);
    const float* samples = buffer.getReadPointer(channel);
    const int numSamples = buffer.getNumSamples();

    for (int i = 0; i < numSamples; ++i)
    {
        ring[static_cast<size_t>(writeIndex)] = samples[i];
        writeIndex = (writeIndex + 1) % analysisSize;
        ++samplesSinceAnalyse;

        if (samplesSinceAnalyse >= hopSize)
        {
            samplesSinceAnalyse = 0;
            analyse();
        }
    }
}

float Tuner::getFrequencyHz() const noexcept
{
    return frequencyHz.load(std::memory_order_relaxed);
}

float Tuner::getCentsOffset() const noexcept
{
    return centsOffset.load(std::memory_order_relaxed);
}

bool Tuner::isLocked() const noexcept
{
    return locked.load(std::memory_order_relaxed);
}

juce::String Tuner::getNoteName() const
{
    const float hz = frequencyHz.load(std::memory_order_relaxed);
    if (hz <= 0.0f || !locked.load(std::memory_order_relaxed))
        return "--";

    const int midi = juce::roundToInt(69.0f + 12.0f * std::log2(hz / 440.0f));
    return midiNoteToName(midi);
}

void Tuner::analyse()
{
    float energy = 0.0f;
    for (float sample : ring)
        energy += sample * sample;
    energy = std::sqrt(energy / static_cast<float>(analysisSize));

    if (energy < silenceThreshold)
    {
        locked.store(false, std::memory_order_relaxed);
        frequencyHz.store(0.0f, std::memory_order_relaxed);
        centsOffset.store(0.0f, std::memory_order_relaxed);
        return;
    }

    const int minLag = juce::jmax(1, static_cast<int>(currentSampleRate / maxFrequencyHz));
    const int maxLag = juce::jmin(analysisSize - 1, static_cast<int>(currentSampleRate / minFrequencyHz));

    float bestCorr = 0.0f;
    int bestLag = 0;
    float zeroLag = 0.0f;

    for (int i = 0; i < analysisSize; ++i)
        zeroLag += ring[static_cast<size_t>(i)] * ring[static_cast<size_t>(i)];

    if (zeroLag <= 1.0e-12f)
    {
        locked.store(false, std::memory_order_relaxed);
        return;
    }

    for (int lag = minLag; lag <= maxLag; ++lag)
    {
        float corr = 0.0f;
        for (int i = 0; i < analysisSize - lag; ++i)
            corr += ring[static_cast<size_t>(i)] * ring[static_cast<size_t>(i + lag)];

        if (corr > bestCorr)
        {
            bestCorr = corr;
            bestLag = lag;
        }
    }

    const float confidence = bestCorr / zeroLag;
    if (bestLag <= 0 || confidence < lockThreshold)
    {
        locked.store(false, std::memory_order_relaxed);
        return;
    }

    float refinedLag = static_cast<float>(bestLag);
    if (bestLag > minLag && bestLag < maxLag)
    {
        auto corrAt = [&](int lag) {
            float corr = 0.0f;
            for (int i = 0; i < analysisSize - lag; ++i)
                corr += ring[static_cast<size_t>(i)] * ring[static_cast<size_t>(i + lag)];
            return corr;
        };

        const float y0 = corrAt(bestLag - 1);
        const float y1 = corrAt(bestLag);
        const float y2 = corrAt(bestLag + 1);
        const float denom = 2.0f * (2.0f * y1 - y0 - y2);
        if (std::abs(denom) > 1.0e-12f)
            refinedLag += (y0 - y2) / denom;
    }

    const float hz = static_cast<float>(currentSampleRate / static_cast<double>(refinedLag));
    if (hz < minFrequencyHz || hz > maxFrequencyHz)
    {
        locked.store(false, std::memory_order_relaxed);
        return;
    }

    const float midiFloat = 69.0f + 12.0f * std::log2(hz / 440.0f);
    const int nearest = juce::roundToInt(midiFloat);
    const float cents = (midiFloat - static_cast<float>(nearest)) * 100.0f;

    frequencyHz.store(hz, std::memory_order_relaxed);
    centsOffset.store(cents, std::memory_order_relaxed);
    locked.store(true, std::memory_order_relaxed);
}

juce::String Tuner::midiNoteToName(int midiNote)
{
    static constexpr const char* names[] = {
        "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"
    };
    const int clamped = juce::jlimit(0, 127, midiNote);
    const int pitchClass = clamped % 12;
    const int octave = (clamped / 12) - 1;
    return juce::String(names[pitchClass]) + juce::String(octave);
}

} // namespace lumen::audio
