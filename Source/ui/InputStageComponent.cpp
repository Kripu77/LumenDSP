#include "ui/InputStageComponent.h"

namespace lumen::ui
{

InputStageComponent::InputStageComponent()
    : gatePedal("NOISE GATE", "PRE-AMP", PedalModuleComponent::Finish::charcoal)
{
    headlineLabel.setText("Input & Dynamics", juce::dontSendNotification);
    headlineLabel.setFont(design::titleFont());
    headlineLabel.setColour(juce::Label::textColourId, design::textPrimary());
    addAndMakeVisible(headlineLabel);

    bodyLabel.setText(
        "Set interface gain first. Use INPUT in the control bar for drive into the model. GATE cleans idle noise before the amp.",
        juce::dontSendNotification);
    bodyLabel.setFont(design::bodyFont());
    bodyLabel.setColour(juce::Label::textColourId, design::textSecondary());
    addAndMakeVisible(bodyLabel);

    addAndMakeVisible(gatePedal);
}

void InputStageComponent::paint(juce::Graphics& graphics)
{
    design::drawCard(graphics, getLocalBounds().toFloat(), 12.0f);
}

void InputStageComponent::resized()
{
    auto bounds = getLocalBounds().reduced(24, 20);
    headlineLabel.setBounds(bounds.removeFromTop(24));
    bodyLabel.setBounds(bounds.removeFromTop(40));
    bounds.removeFromTop(8);
    gatePedal.setBounds(bounds.withSizeKeepingCentre(280, juce::jmin(380, bounds.getHeight())));
}

PedalModuleComponent& InputStageComponent::getGatePedal() noexcept
{
    return gatePedal;
}

} // namespace lumen::ui
