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

void Drive::process(juce::AudioBuffer<float>& buffer)
{
    if (!prepared || !enabled)
        return;

    if (std::abs(tone - appliedTone) > 0.001f)
        updateToneFilter();

    const float preGain = 1.0f + drive * 18.0f;
    const float shape = 1.2f + drive * 2.5f;
    const float outputGain = 0.25f + level * 0.85f;

    for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
    {
        auto* samples = buffer.getWritePointer(ch);
        for (int i = 0; i < buffer.getNumSamples(); ++i)
        {
            float x = samples[i] * preGain;
            x = std::tanh(x * shape) / std::tanh(shape);
            samples[i] = x * outputGain;
        }
    }

    juce::dsp::AudioBlock<float> block(buffer);
    juce::dsp::ProcessContextReplacing<float> context(block);
    toneFilter.process(context);
}

} // namespace lumen::audio
