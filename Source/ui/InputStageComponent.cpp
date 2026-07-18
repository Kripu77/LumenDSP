#include "ui/InputStageComponent.h"

namespace lumen::ui
{

InputStageComponent::InputStageComponent()
    : gatePedal("NOISE GATE", "PRE-AMP DYNAMICS", PedalModuleComponent::Finish::steel)
{
    headlineLabel.setText("INPUT STAGE", juce::dontSendNotification);
    headlineLabel.setFont(design::titleFont());
    headlineLabel.setColour(juce::Label::textColourId, design::chromeTextPrimary());
    headlineLabel.setJustificationType(juce::Justification::centredLeft);
    addAndMakeVisible(headlineLabel);

    bodyLabel.setText(
        "Gain-stage at your interface first. Chrome INPUT sets drive into the model. GATE cleans idle hiss before the amp.",
        juce::dontSendNotification);
    bodyLabel.setFont(design::bodyFont());
    bodyLabel.setColour(juce::Label::textColourId, design::chromeTextSecondary());
    bodyLabel.setJustificationType(juce::Justification::centredLeft);
    addAndMakeVisible(bodyLabel);

    addAndMakeVisible(gatePedal);
}

void InputStageComponent::paint(juce::Graphics& graphics)
{
    auto bounds = getLocalBounds().toFloat().reduced(10.0f);
    design::drawSoftShadow(graphics, bounds, static_cast<float>(design::panelCornerRadiusPixels), 0.14f);

    juce::ColourGradient board(
        juce::Colour::fromRGB(214, 220, 228),
        bounds.getX(),
        bounds.getY(),
        juce::Colour::fromRGB(186, 194, 204),
        bounds.getX(),
        bounds.getBottom(),
        false);
    graphics.setGradientFill(board);
    graphics.fillRoundedRectangle(bounds, static_cast<float>(design::panelCornerRadiusPixels));
    graphics.setColour(design::chromeBorder());
    graphics.drawRoundedRectangle(bounds, static_cast<float>(design::panelCornerRadiusPixels), 1.0f);

    auto carpet = bounds.reduced(28.0f, 70.0f);
    graphics.setColour(juce::Colour::fromRGB(62, 68, 76).withAlpha(0.18f));
    graphics.fillRoundedRectangle(carpet, 18.0f);

    for (float y = carpet.getY() + 18.0f; y < carpet.getBottom(); y += 22.0f)
    {
        graphics.setColour(juce::Colours::white.withAlpha(0.04f));
        graphics.drawLine(carpet.getX() + 16.0f, y, carpet.getRight() - 16.0f, y, 1.0f);
    }

    graphics.setColour(design::chromeTextMuted());
    graphics.setFont(design::microFont());
    graphics.drawText(
        "PEDALBOARD",
        bounds.removeFromBottom(28.0f).reduced(32.0f, 0.0f),
        juce::Justification::centredLeft);
}

void InputStageComponent::resized()
{
    auto bounds = getLocalBounds().reduced(36, 28);
    headlineLabel.setBounds(bounds.removeFromTop(26));
    bodyLabel.setBounds(bounds.removeFromTop(40));
    bounds.removeFromTop(8);

    const int pedalWidth = juce::jlimit(240, 300, bounds.getWidth() / 3);
    const int pedalHeight = juce::jlimit(320, 420, bounds.getHeight() - 12);
    gatePedal.setBounds(bounds.withSizeKeepingCentre(pedalWidth, pedalHeight));
}

PedalModuleComponent& InputStageComponent::getGatePedal() noexcept
{
    return gatePedal;
}

} // namespace lumen::ui
