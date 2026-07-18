#include "audio/ReverbFx.h"

namespace lumen::audio
{

void ReverbFx::prepare(const juce::dsp::ProcessSpec& processSpec)
{
    reverb.prepare(processSpec);
    prepared = true;
    applyParameters();
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

void ReverbFx::setCharacter(ReverbCharacter newCharacter) noexcept
{
    character = newCharacter;
    applyParameters();
}

void ReverbFx::setRoomSize(float size01) noexcept
{
    size = juce::jlimit(0.0f, 1.0f, size01);
    applyParameters();
}

void ReverbFx::setDamping(float damping01) noexcept
{
    damping = juce::jlimit(0.0f, 1.0f, damping01);
    applyParameters();
}

void ReverbFx::setMix(float mix01) noexcept
{
    mix = juce::jlimit(0.0f, 1.0f, mix01);
    applyParameters();
}

void ReverbFx::applyParameters()
{
    float sizeBias = 0.0f;
    float dampBias = 0.0f;
    float width = 1.0f;

    switch (character)
    {
        case ReverbCharacter::hall:
            sizeBias = 0.25f;
            dampBias = -0.15f;
            width = 1.0f;
            break;
        case ReverbCharacter::plate:
            sizeBias = 0.05f;
            dampBias = -0.28f;
            width = 0.85f;
            break;
        case ReverbCharacter::ambient:
            sizeBias = 0.35f;
            dampBias = 0.2f;
            width = 1.0f;
            break;
        case ReverbCharacter::room:
        default:
            sizeBias = -0.1f;
            dampBias = 0.05f;
            width = 0.7f;
            break;
    }

    params.roomSize = juce::jlimit(0.0f, 1.0f, size + sizeBias);
    params.damping = juce::jlimit(0.0f, 1.0f, damping + dampBias);
    params.width = width;
    params.freezeMode = 0.0f;
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
