#include "audio/ReverbFx.h"

namespace lumen::audio
{

void ReverbFx::prepare(const juce::dsp::ProcessSpec& processSpec)
{
    reverb.prepare(processSpec);
    params.roomSize = 0.4f;
    params.damping = 0.5f;
    params.width = 1.0f;
    params.freezeMode = 0.0f;
    params.wetLevel = 0.18f;
    params.dryLevel = 0.82f;
    reverb.setParameters(params);
    prepared = true;
    reset();
}

void ReverbFx::reset()
{
    reverb.reset();
}

void ReverbFx::setEnabled(bool shouldEnable) noexcept
{
    enabled = shouldEnable;
}

void ReverbFx::setRoomSize(float size01) noexcept
{
    params.roomSize = juce::jlimit(0.0f, 1.0f, size01);
    applyParameters();
}

void ReverbFx::setDamping(float damping01) noexcept
{
    params.damping = juce::jlimit(0.0f, 1.0f, damping01);
    applyParameters();
}

void ReverbFx::setMix(float mix01) noexcept
{
    mix = juce::jlimit(0.0f, 1.0f, mix01);
    applyParameters();
}

void ReverbFx::applyParameters()
{
    params.wetLevel = mix;
    params.dryLevel = 1.0f - mix;
    if (prepared)
        reverb.setParameters(params);
}

void ReverbFx::process(juce::AudioBuffer<float>& buffer)
{
    if (!prepared || !enabled)
        return;

    juce::dsp::AudioBlock<float> block(buffer);
    juce::dsp::ProcessContextReplacing<float> context(block);
    reverb.process(context);
}

} // namespace lumen::audio
