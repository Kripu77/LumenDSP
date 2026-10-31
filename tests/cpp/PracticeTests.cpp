// Unit tests for the JUCE-free practice capture primitives.
// Kept dependency-free (no test framework) so they build in seconds on every CI runner.

#include "practice/OnsetDetector.h"
#include "practice/PracticeEventQueue.h"

#include <cmath>
#include <cstdint>
#include <cstdio>
#include <functional>
#include <string>
#include <vector>

namespace
{

using lumen::practice::OnsetDetector;
using lumen::practice::PracticeEvent;
using lumen::practice::PracticeEventKind;
using lumen::practice::PracticeEventQueue;

constexpr double sampleRate = 48000.0;
constexpr double pi = 3.14159265358979323846;
// A real pick attack is detected a little after the first sample; 3 ms is well inside the
// ~10 ms where timing errors start to be audible.
constexpr double onsetToleranceMs = 3.0;

int failures = 0;
// Defaults under test; a tuning harness can include this file and vary them.
lumen::practice::OnsetDetectorSettings detectorSettingsUnderTest{};

void expect(bool condition, const std::string& message)
{
    if (!condition)
    {
        ++failures;
        std::printf("  FAIL: %s\n", message.c_str());
    }
}

void runTest(const char* name, const std::function<void()>& test)
{
    const int failuresBefore = failures;
    test();
    std::printf("%s %s\n", failures == failuresBefore ? "ok  " : "FAIL", name);
}

// Deterministic noise so tests never flake.
struct NoiseSource
{
    uint32_t state = 0x12345678u;
    float next()
    {
        state = state * 1664525u + 1013904223u;
        return static_cast<float>(state >> 8) / static_cast<float>(1u << 24) * 2.0f - 1.0f;
    }
};

// A plucked string: a short noise transient plus decaying harmonics.
void addPluck(std::vector<float>& signal, size_t start, double frequencyHz, float amplitude, NoiseSource& noise)
{
    constexpr double decaySeconds = 0.6;
    constexpr int harmonicCount = 5;
    constexpr size_t transientSamples = 96;

    for (size_t index = start; index < signal.size(); ++index)
    {
        const double time = static_cast<double>(index - start) / sampleRate;
        const double envelope = std::exp(-time / decaySeconds);
        double tone = 0.0;
        for (int harmonic = 1; harmonic <= harmonicCount; ++harmonic)
            tone += std::sin(2.0 * pi * frequencyHz * harmonic * time) / harmonic;

        float sample = static_cast<float>(tone * envelope * 0.5);
        if (index - start < transientSamples)
            sample += noise.next() * 0.6f;

        signal[index] += sample * amplitude;
    }
}

std::vector<int64_t> detectOnsets(const std::vector<float>& signal, int blockSize)
{
    OnsetDetector detector;
    detector.prepare(sampleRate, detectorSettingsUnderTest);
    std::vector<int64_t> onsets;

    for (size_t blockStart = 0; blockStart < signal.size(); blockStart += static_cast<size_t>(blockSize))
    {
        const int count = static_cast<int>(std::min<size_t>(static_cast<size_t>(blockSize), signal.size() - blockStart));
        detector.process(signal.data() + blockStart, count, [&](int offset, float) {
            onsets.push_back(static_cast<int64_t>(blockStart) + offset);
        });
    }
    return onsets;
}

double samplesToMs(int64_t samples)
{
    return static_cast<double>(samples) * 1000.0 / sampleRate;
}

size_t secondsToSamples(double seconds)
{
    return static_cast<size_t>(seconds * sampleRate);
}

void testSilenceProducesNoOnsets()
{
    const std::vector<float> silence(secondsToSamples(2.0), 0.0f);
    expect(detectOnsets(silence, 256).empty(), "silence should not trigger");
}

void testQuietNoiseStaysBelowFloor()
{
    NoiseSource noise;
    std::vector<float> hiss(secondsToSamples(2.0));
    for (float& sample : hiss)
        sample = noise.next() * 0.002f;
    expect(detectOnsets(hiss, 256).empty(), "low-level hiss should not trigger");
}

void testDetectsEveryPickOnTime()
{
    // Eighth notes at 120 BPM: one pick every 250 ms.
    constexpr double intervalSeconds = 0.25;
    constexpr int pickCount = 12;
    NoiseSource noise;
    std::vector<float> signal(secondsToSamples(intervalSeconds * (pickCount + 2)));
    std::vector<size_t> expected;
    for (int pick = 0; pick < pickCount; ++pick)
    {
        const size_t start = secondsToSamples(0.1 + intervalSeconds * pick);
        expected.push_back(start);
        addPluck(signal, start, 196.0, 0.7f, noise);
    }

    const auto onsets = detectOnsets(signal, 256);
    expect(onsets.size() == expected.size(),
           "expected " + std::to_string(expected.size()) + " onsets, got " + std::to_string(onsets.size()));

    for (size_t index = 0; index < std::min(onsets.size(), expected.size()); ++index)
    {
        const double errorMs = samplesToMs(onsets[index] - static_cast<int64_t>(expected[index]));
        expect(errorMs >= 0.0 && errorMs <= onsetToleranceMs,
               "pick " + std::to_string(index) + " detected " + std::to_string(errorMs) + " ms off");
    }
}

void testRepickWhileStringStillRings()
{
    // Second pick lands while the first note is still loud: the case a plain gate would miss.
    NoiseSource noise;
    std::vector<float> signal(secondsToSamples(1.0));
    addPluck(signal, secondsToSamples(0.1), 220.0, 0.8f, noise);
    addPluck(signal, secondsToSamples(0.25), 220.0, 0.8f, noise);
    expect(detectOnsets(signal, 128).size() == 2, "re-pick over a ringing note should be detected");
}

void testSoftPickAfterLoudPick()
{
    // Dynamics matter: a soft pick following an accent must not be swallowed.
    NoiseSource noise;
    std::vector<float> signal(secondsToSamples(1.0));
    addPluck(signal, secondsToSamples(0.1), 196.0, 1.0f, noise);
    addPluck(signal, secondsToSamples(0.35), 196.0, 0.3f, noise);
    expect(detectOnsets(signal, 256).size() == 2, "soft pick after a loud one should be detected");
}

void testFastSixteenthNotes()
{
    // 16ths at 140 BPM are ~107 ms apart: typical fusion picking runs.
    constexpr double intervalSeconds = 60.0 / 140.0 / 4.0;
    constexpr int pickCount = 16;
    NoiseSource noise;
    std::vector<float> signal(secondsToSamples(intervalSeconds * (pickCount + 4)));
    for (int pick = 0; pick < pickCount; ++pick)
        addPluck(signal, secondsToSamples(0.05 + intervalSeconds * pick), 330.0, 0.6f, noise);

    const auto onsets = detectOnsets(signal, 256);
    expect(onsets.size() == pickCount,
           "expected 16 onsets at 140 BPM 16ths, got " + std::to_string(onsets.size()));
}

void testSustainedToneDoesNotRetrigger()
{
    std::vector<float> signal(secondsToSamples(2.0));
    for (size_t index = 0; index < signal.size(); ++index)
        signal[index] = 0.5f * static_cast<float>(std::sin(2.0 * pi * 330.0 * static_cast<double>(index) / sampleRate));
    expect(detectOnsets(signal, 256).size() == 1, "a steady tone should only trigger at its start");
}

void testRefractoryMergesDoubleTrigger()
{
    // Two transients 10 ms apart are one pick (e.g. pick + string slap), not two.
    NoiseSource noise;
    std::vector<float> signal(secondsToSamples(1.0));
    addPluck(signal, secondsToSamples(0.1), 196.0, 0.7f, noise);
    addPluck(signal, secondsToSamples(0.11), 196.0, 0.7f, noise);
    expect(detectOnsets(signal, 256).size() == 1, "transients inside the refractory window should merge");
}

void testBlockSizeDoesNotChangeResults()
{
    NoiseSource noise;
    std::vector<float> signal(secondsToSamples(2.0));
    for (int pick = 0; pick < 6; ++pick)
        addPluck(signal, secondsToSamples(0.1 + 0.3 * pick), 147.0, 0.6f, noise);

    const auto small = detectOnsets(signal, 32);
    const auto large = detectOnsets(signal, 1024);
    const auto odd = detectOnsets(signal, 333);
    expect(small == large && large == odd, "onset positions must not depend on audio buffer size");
}

void testZeroLengthBlockIsHarmless()
{
    OnsetDetector detector;
    detector.prepare(sampleRate);
    int calls = 0;
    detector.process(nullptr, 0, [&](int, float) { ++calls; });
    expect(calls == 0, "empty block should not call back");
}

void testQueuePreservesOrder()
{
    PracticeEventQueue<8> queue;
    for (int index = 0; index < 5; ++index)
        expect(queue.push(PracticeEvent{PracticeEventKind::onset, index, 0.0f, 0.0f}), "push should succeed");

    PracticeEvent out[8];
    const size_t count = queue.pop(out, 8);
    expect(count == 5, "should pop all queued events");
    for (size_t index = 0; index < count; ++index)
        expect(out[index].sampleIndex == static_cast<int64_t>(index), "events must come out in order");
}

void testQueueDropsWhenFullAndCounts()
{
    PracticeEventQueue<4> queue;
    for (int index = 0; index < 4; ++index)
        queue.push(PracticeEvent{PracticeEventKind::beat, index, 0.0f, 0.0f});

    expect(!queue.push(PracticeEvent{}), "push into a full queue must fail, not overwrite");
    expect(queue.droppedCount() == 1, "dropped events should be counted");

    PracticeEvent out[4];
    expect(queue.pop(out, 4) == 4 && out[0].sampleIndex == 0, "oldest events must survive overflow");
}

void testQueueWrapsAround()
{
    PracticeEventQueue<4> queue;
    PracticeEvent out[4];
    for (int round = 0; round < 10; ++round)
    {
        queue.push(PracticeEvent{PracticeEventKind::pitch, round * 2, 0.0f, 0.0f});
        queue.push(PracticeEvent{PracticeEventKind::pitch, round * 2 + 1, 0.0f, 0.0f});
        const size_t count = queue.pop(out, 4);
        expect(count == 2 && out[0].sampleIndex == round * 2 && out[1].sampleIndex == round * 2 + 1,
               "wrap-around round " + std::to_string(round));
    }
}

void testQueuePartialPopAndClear()
{
    PracticeEventQueue<8> queue;
    for (int index = 0; index < 6; ++index)
        queue.push(PracticeEvent{PracticeEventKind::onset, index, 0.0f, 0.0f});

    PracticeEvent out[8];
    expect(queue.pop(out, 2) == 2 && out[1].sampleIndex == 1, "partial pop returns the oldest events");

    queue.clear();
    expect(queue.pop(out, 8) == 0, "clear should discard the rest");
    expect(queue.push(PracticeEvent{}), "queue is usable after clear");
}

} // namespace

int main()
{
    runTest("silence produces no onsets", testSilenceProducesNoOnsets);
    runTest("quiet noise stays below floor", testQuietNoiseStaysBelowFloor);
    runTest("detects every pick on time", testDetectsEveryPickOnTime);
    runTest("re-pick while string still rings", testRepickWhileStringStillRings);
    runTest("soft pick after loud pick", testSoftPickAfterLoudPick);
    runTest("fast sixteenth notes", testFastSixteenthNotes);
    runTest("sustained tone does not retrigger", testSustainedToneDoesNotRetrigger);
    runTest("refractory merges double trigger", testRefractoryMergesDoubleTrigger);
    runTest("block size does not change results", testBlockSizeDoesNotChangeResults);
    runTest("zero-length block is harmless", testZeroLengthBlockIsHarmless);
    runTest("queue preserves order", testQueuePreservesOrder);
    runTest("queue drops when full and counts", testQueueDropsWhenFullAndCounts);
    runTest("queue wraps around", testQueueWrapsAround);
    runTest("queue partial pop and clear", testQueuePartialPopAndClear);

    std::printf("\n%s (%d failure%s)\n", failures == 0 ? "PASS" : "FAIL", failures, failures == 1 ? "" : "s");
    return failures == 0 ? 0 : 1;
}
