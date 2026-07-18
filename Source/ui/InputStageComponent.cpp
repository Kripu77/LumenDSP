#include "ui/InputStageComponent.h"

namespace lumen::ui
{

InputStageComponent::InputStageComponent()
    : gatePedal("NOISE GATE", "PRE-AMP DYNAMICS", PedalModuleComponent::Finish::steel)
{
    headlineLabel.setText("INPUT STAGE", juce::dontSendNotification);
    headlineLabel.setFont(design::titleFont());
    headlineLabel.setColour(juce::Label::textColourId, design::chromeTextPrimary());
    headlineLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(headlineLabel);

    bodyLabel.setText(
        "Set interface gain first. Use INPUT and GATE in the chrome bar, then engage the gate pedal for idle noise.",
        juce::dontSendNotification);
    bodyLabel.setFont(design::bodyFont());
    bodyLabel.setColour(juce::Label::textColourId, design::chromeTextSecondary());
    bodyLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(bodyLabel);

    addAndMakeVisible(gatePedal);
}

void InputStageComponent::paint(juce::Graphics& graphics)
{
    auto bounds = getLocalBounds().toFloat().reduced(24.0f);
    design::drawChromePanel(graphics, bounds, static_cast<float>(design::panelCornerRadiusPixels));
}

void InputStageComponent::resized()
{
    auto bounds = getLocalBounds().reduced(48, 40);
    headlineLabel.setBounds(bounds.removeFromTop(28));
    bodyLabel.setBounds(bounds.removeFromTop(48));
    bounds.removeFromTop(12);
    gatePedal.setBounds(bounds.withSizeKeepingCentre(260, juce::jmin(360, bounds.getHeight())));
}

PedalModuleComponent& InputStageComponent::getGatePedal() noexcept
{
    return gatePedal;
}

} // namespace lumen::ui
