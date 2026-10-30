#pragma once

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>

namespace lumen::practice
{

// Detects picked-note attacks in a mono DI signal.
//
// Deliberately free of JUCE so it can be unit tested with a plain C++ compiler.
// It runs on the audio thread: no allocation, no locks, O(1) per sample.
//
// Approach: high-pass the signal steeply (cascaded poles) so the broadband pick
// transient dominates and the ringing note's fundamental and low harmonics mostly drop
// out of the detection band. Then compare a fast power envelope (tracks the attack)
// against a slow power envelope (tracks the ringing background). An onset fires when
// fast power jumps well above slow power.
//
// Power (mean-square) rather than peak envelopes matters: a peak follower ripples on
// every cycle of a ringing note, which forces a high threshold and drops picks in dense
// runs. Smoothed power of a steady tone is nearly flat, so the threshold can sit low.
// A refractory window plus a re-arm hysteresis stop one pick from being reported twice.
//
// Legato notes (hammer-ons, pull-offs, slides) have little transient energy
// and are often not reported. That is acceptable: timing analysis is about
// where the pick lands.
struct OnsetDetectorSettings
{
    // Guitar fundamentals top out near 1.3 kHz; pick noise extends well above that.
    float highPassHz = 1500.0f;
    // Number of cascaded one-pole stages (6 dB/octave each), clamped to 1..maxHighPassOrder.
    int highPassOrder = 2;
    float fastAttackMs = 0.5f;
    float fastReleaseMs = 3.0f;
    float slowAttackMs = 30.0f;
    float slowReleaseMs = 120.0f;
    // Power ratio (fast / slow) that counts as an attack. 4x power is +6 dB.
    float triggerRatio = 4.0f;
    // Fast power must fall back below slow * rearmRatio before the next onset can fire.
    float rearmRatio = 2.0f;
    // Ignores noise and string squeaks quieter than this (linear amplitude after high-pass).
    float absoluteFloor = 0.008f;
    // Even very fast 32nd-note picking at 200 BPM is ~37 ms apart, so 30 ms is safe.
    float refractoryMs = 30.0f;
};

class OnsetDetector
{
public:
    void prepare(double sampleRate, const OnsetDetectorSettings& newSettings = {})
    {
        settings = newSettings;
        currentSampleRate = sampleRate > 0.0 ? sampleRate : defaultSampleRate;

        highPassCoefficient = onePoleHighPassCoefficient(settings.highPassHz);
        activeHighPassStages = std::clamp(settings.highPassOrder, 1, maxHighPassOrder);
        fastAttack = smoothingCoefficient(settings.fastAttackMs);
        fastRelease = smoothingCoefficient(settings.fastReleaseMs);
        slowAttack = smoothingCoefficient(settings.slowAttackMs);
        slowRelease = smoothingCoefficient(settings.slowReleaseMs);
        absoluteFloorPower = settings.absoluteFloor * settings.absoluteFloor;
        refractorySamples = static_cast<int64_t>(currentSampleRate * settings.refractoryMs / millisecondsPerSecond);
        reset();
    }

    void reset() noexcept
    {
        highPassStages = {};
        fastEnvelope = 0.0f;
        slowEnvelope = 0.0f;
        isArmed = true;
        samplesSinceOnset = refractorySamples;
    }

    // Calls onOnset(sampleOffsetWithinBlock, strength) for each detected attack.
    // strength is the fast/slow power ratio at the moment of detection, useful for UI weighting.
    template <typename OnsetCallback>
    void process(const float* samples, int numSamples, OnsetCallback&& onOnset)
    {
        for (int index = 0; index < numSamples; ++index)
        {
            const float highPassed = highPass(samples[index]);
            const float power = highPassed * highPassed;

            fastEnvelope = follow(fastEnvelope, power, fastAttack, fastRelease);
            slowEnvelope = follow(slowEnvelope, power, slowAttack, slowRelease);

            if (samplesSinceOnset < refractorySamples)
                ++samplesSinceOnset;

            if (!isArmed && fastEnvelope < slowEnvelope * settings.rearmRatio)
                isArmed = true;

            const bool isLoudEnough = fastEnvelope > absoluteFloorPower;
            const bool isRisingSharply = fastEnvelope > slowEnvelope * settings.triggerRatio;
            const bool isOutsideRefractory = samplesSinceOnset >= refractorySamples;

            if (isArmed && isLoudEnough && isRisingSharply && isOutsideRefractory)
            {
                const float strength = slowEnvelope > minimumEnvelope ? fastEnvelope / slowEnvelope
                                                                      : settings.triggerRatio;
                onOnset(index, strength);
                isArmed = false;
                samplesSinceOnset = 0;
            }
        }
    }

private:
    static constexpr double defaultSampleRate = 48000.0;
    static constexpr double millisecondsPerSecond = 1000.0;
    static constexpr double twoPi = 6.283185307179586;
    static constexpr float minimumEnvelope = 1.0e-12f;

    struct HighPassStage
    {
        float previousInput = 0.0f;
        float previousOutput = 0.0f;

        // One-pole high-pass: y[n] = a * (y[n-1] + x[n] - x[n-1]).
        float process(float input, float coefficient) noexcept
        {
            const float output = coefficient * (previousOutput + input - previousInput);
            previousInput = input;
            previousOutput = output;
            return output;
        }
    };

    float highPass(float input) noexcept
    {
        float output = input;
        for (int stage = 0; stage < activeHighPassStages; ++stage)
            output = highPassStages[static_cast<size_t>(stage)].process(output, highPassCoefficient);
        return output;
    }

    static float follow(float envelope, float target, float attack, float release) noexcept
    {
        const float coefficient = target > envelope ? attack : release;
        return envelope + coefficient * (target - envelope);
    }

    float smoothingCoefficient(float timeMs) const noexcept
    {
        const double timeSamples = std::max(1.0, currentSampleRate * timeMs / millisecondsPerSecond);
        return static_cast<float>(1.0 - std::exp(-1.0 / timeSamples));
    }

    float onePoleHighPassCoefficient(float cutoffHz) const noexcept
    {
        const double rc = 1.0 / (twoPi * std::max(1.0f, cutoffHz));
        const double dt = 1.0 / currentSampleRate;
        return static_cast<float>(rc / (rc + dt));
    }

    OnsetDetectorSettings settings{};
    double currentSampleRate = defaultSampleRate;

    float highPassCoefficient = 0.0f;
    float fastAttack = 0.0f;
    float fastRelease = 0.0f;
    float slowAttack = 0.0f;
    float slowRelease = 0.0f;
    float absoluteFloorPower = 0.0f;
    int64_t refractorySamples = 0;

    static constexpr int maxHighPassOrder = 4;
    std::array<HighPassStage, maxHighPassOrder> highPassStages{};
    int activeHighPassStages = 2;
    float fastEnvelope = 0.0f;
    float slowEnvelope = 0.0f;
    bool isArmed = true;
    int64_t samplesSinceOnset = 0;
};

} // namespace lumen::practice
