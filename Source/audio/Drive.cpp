#include "audio/Drive.h"
#include <cmath>

namespace lumen::audio
{

void Drive::prepare(const juce::dsp::ProcessSpec& processSpec)
{
    currentSpec = processSpec;
    toneFilter.prepare(processSpec);
    prepared = true;
    appliedTone = -1.0f;
    updateToneFilter();
    reset();
}

void Drive::reset()
{
    toneFilter.reset();
}

void Drive::setEnabled(bool shouldEnable) noexcept
{
    enabled = shouldEnable;
}

void Drive::setMode(DriveMode newMode) noexcept
{
    mode = newMode;
}

void Drive::setDrive(float drive01) noexcept
{
    drive = juce::jlimit(0.0f, 1.0f, drive01);
}

void Drive::setTone(float tone01) noexcept
{
    tone = juce::jlimit(0.0f, 1.0f, tone01);
}

void Drive::setLevel(float level01) noexcept
{
    level = juce::jlimit(0.0f, 1.0f, level01);
}

void Drive::updateToneFilter()
{
    if (!prepared)
        return;

    const float frequency = 800.0f + tone * 4200.0f;
    *toneFilter.state = *Coefficients::makeLowPass(currentSpec.sampleRate, frequency, 0.7f);
    appliedTone = tone;
}

float Drive::shapeSample(float sample, DriveMode mode, float driveAmount)
{
    switch (mode)
    {
        case DriveMode::hard:
        {
            const float preGain = 1.0f + driveAmount * 24.0f;
            return juce::jlimit(-1.0f, 1.0f, sample * preGain);
        }
        case DriveMode::tube:
        {
            const float preGain = 1.0f + driveAmount * 16.0f;
            float x = sample * preGain;
            if (x >= 0.0f)
                return std::tanh(x * 1.4f) * 0.92f;
            return std::tanh(x * 2.1f) * 0.78f;
        }
        case DriveMode::boost:
        {
            const float preGain = 1.0f + driveAmount * 8.0f;
            const float x = sample * preGain;
            return std::tanh(x * 0.85f) * 0.95f;
        }
        case DriveMode::soft:
        default:
        {
            const float preGain = 1.0f + driveAmount * 18.0f;
            const float shape = 1.2f + driveAmount * 2.5f;
            const float x = sample * preGain;
            return std::tanh(x * shape) / std::tanh(shape);
        }
    }
}

void Drive::process(juce::AudioBuffer<float>& buffer)
{
    if (!prepared || !enabled)
        return;

    if (std::abs(tone - appliedTone) > 0.001f)
        updateToneFilter();

    const float outputGain = 0.25f + level * 0.85f;

    for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
    {
        auto* samples = buffer.getWritePointer(ch);
        for (int i = 0; i < buffer.getNumSamples(); ++i)
            samples[i] = shapeSample(samples[i], mode, drive) * outputGain;
    }

    juce::dsp::AudioBlock<float> block(buffer);
    juce::dsp::ProcessContextReplacing<float> context(block);
    toneFilter.process(context);
}

} // namespace lumen::audio
