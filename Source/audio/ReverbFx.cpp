#include "audio/ReverbFx.h"

namespace lumen::audio
{

void ReverbFx::prepare(const juce::dsp::ProcessSpec& processSpec)
{
    maximumBlockSize = static_cast<int>(processSpec.maximumBlockSize);
    reverb.configure(processSpec.sampleRate, processSpec.maximumBlockSize, 2);
    reverb.dry = 0.0;
    reverb.wet = 1.0;
    wet.setSize(2, maximumBlockSize);
    wetGain.reset(processSpec.sampleRate, 0.05);
    width.reset(processSpec.sampleRate, 0.05);
    prepared = true;
    setCharacter(character);
    reset();
}

void ReverbFx::reset()
{
    reverb.reset();
}

void ReverbFx::setEnabled(bool shouldEnable) noexcept
{
    if (shouldEnable && !enabled && prepared)
        reset();
    enabled = shouldEnable;
}

void ReverbFx::setCharacter(ReverbCharacter newCharacter) noexcept
{
    character = newCharacter;
    struct Shape { double roomMs, early, detune, lowDamp, highDamp; };
    // Room is small and dark, plate dense and bright, ambient wide and slowly detuned.
    static constexpr Shape shapes[] = {
        { 30.0, 1.2, 1.0, 1.5, 3.0 },
        { 90.0, 1.0, 3.0, 1.5, 2.5 },
        { 45.0, 0.3, 4.0, 1.2, 1.6 },
        { 120.0, 0.55, 8.5, 1.5, 2.0 },
    };
    const auto& shape = shapes[juce::jlimit(0, 3, static_cast<int>(newCharacter))];
    reverb.roomMs = shape.roomMs;
    reverb.early = shape.early;
    reverb.detune = shape.detune;
    reverb.lowDampRate = shape.lowDamp;
    reverb.highDampRate = shape.highDamp;
}

void ReverbFx::setDecaySeconds(float seconds) noexcept
{
    // rt20 is the time to fall 20 dB; x1.13 makes the measured RT60 match the knob (damping shortens it).
    reverb.rt20 = juce::jlimit(0.1f, 30.0f, seconds) / 3.0f * 1.13f;
}

void ReverbFx::setLowCutHz(float hz) noexcept
{
    reverb.lowCutHz = juce::jlimit(10.0f, 1000.0f, hz);
}

void ReverbFx::setHighCutHz(float hz) noexcept
{
    reverb.highCutHz = juce::jlimit(500.0f, 20000.0f, hz);
}

void ReverbFx::setWidth(float width01) noexcept
{
    width.setTargetValue(juce::jlimit(0.0f, 1.0f, width01));
}

void ReverbFx::setMix(float mix01) noexcept
{
    // Dry stays at unity; mix 0.25 gives the library's default wet balance.
    wetGain.setTargetValue(2.0f * juce::jlimit(0.0f, 1.0f, mix01));
}

void ReverbFx::process(juce::AudioBuffer<float>& buffer)
{
    if (!prepared || !enabled || buffer.getNumChannels() == 0)
        return;

    const bool stereo = buffer.getNumChannels() > 1;
    for (int start = 0; start < buffer.getNumSamples(); start += maximumBlockSize)
    {
        const int count = juce::jmin(maximumBlockSize, buffer.getNumSamples() - start);
        float* left = buffer.getWritePointer(0, start);
        float* right = stereo ? buffer.getWritePointer(1, start) : left;
        const float* inputs[] = { left, right };
        float* outputs[] = { wet.getWritePointer(0), wet.getWritePointer(1) };
        reverb.process(inputs, outputs, static_cast<size_t>(count));

        for (int i = 0; i < count; ++i)
        {
            const float gain = wetGain.getNextValue();
            const float mid = 0.5f * (outputs[0][i] + outputs[1][i]);
            const float side = 0.5f * (outputs[0][i] - outputs[1][i]) * width.getNextValue();
            left[i] += gain * (mid + side);
            if (stereo)
                right[i] += gain * (mid - side);
        }
    }
}

} // namespace lumen::audio
