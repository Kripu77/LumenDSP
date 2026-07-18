#include "ui/TopChromeComponent.h"

namespace lumen::ui
{

TopChromeComponent::TopChromeComponent()
{
    brandLabel.setText("LUMEN", juce::dontSendNotification);
    brandLabel.setFont(design::brandFont());
    brandLabel.setColour(juce::Label::textColourId, design::accent());
    brandLabel.setJustificationType(juce::Justification::centredLeft);
    addAndMakeVisible(brandLabel);

    productLabel.setText("DSP", juce::dontSendNotification);
    productLabel.setFont(design::titleFont());
    productLabel.setColour(juce::Label::textColourId, design::textPrimary());
    productLabel.setJustificationType(juce::Justification::centredLeft);
    addAndMakeVisible(productLabel);

    addAndMakeVisible(inputMeter);
    addAndMakeVisible(outputMeter);
    addAndMakeVisible(inputKnob);
    addAndMakeVisible(gateKnob);
    addAndMakeVisible(outputKnob);
    addAndMakeVisible(gateEnableButton);
    addAndMakeVisible(presetBar);
    addAndMakeVisible(audioButton);
}

void TopChromeComponent::paint(juce::Graphics& graphics)
{
    auto bounds = getLocalBounds().toFloat();
    graphics.setColour(design::bgHeader());
    graphics.fillRoundedRectangle(bounds, 10.0f);
    graphics.setColour(design::borderLight());
    graphics.drawRoundedRectangle(bounds.reduced(0.5f), 10.0f, 1.0f);
}

void TopChromeComponent::resized()
{
    auto bounds = getLocalBounds().reduced(12, 8);

    auto brand = bounds.removeFromLeft(78);
    brandLabel.setBounds(brand.removeFromTop(22).withTrimmedTop(2));
    productLabel.setBounds(brand.removeFromTop(20));

    bounds.removeFromLeft(8);
    auto inputCluster = bounds.removeFromLeft(108);
    inputMeter.setBounds(inputCluster.removeFromLeft(14).reduced(1, 6));
    inputKnob.setBounds(inputCluster);

    bounds.removeFromLeft(4);
    auto gateCluster = bounds.removeFromLeft(108);
    gateEnableButton.setBounds(gateCluster.removeFromTop(20).withSizeKeepingCentre(88, 18));
    gateKnob.setBounds(gateCluster);

    auto right = bounds.removeFromRight(190);
    audioButton.setBounds(right.removeFromTop(28).removeFromRight(78));
    right.removeFromTop(2);
    auto outputCluster = right;
    outputMeter.setBounds(outputCluster.removeFromRight(14).reduced(1, 6));
    outputKnob.setBounds(outputCluster);

    bounds.reduce(8, 2);
    presetBar.setBounds(bounds.withSizeKeepingCentre(juce::jmax(260, bounds.getWidth()), 36));
}

KnobComponent& TopChromeComponent::getInputKnob() noexcept { return inputKnob; }
KnobComponent& TopChromeComponent::getGateKnob() noexcept { return gateKnob; }
KnobComponent& TopChromeComponent::getOutputKnob() noexcept { return outputKnob; }
juce::ToggleButton& TopChromeComponent::getGateEnableButton() noexcept { return gateEnableButton; }
LedMeterComponent& TopChromeComponent::getInputMeter() noexcept { return inputMeter; }
LedMeterComponent& TopChromeComponent::getOutputMeter() noexcept { return outputMeter; }
PresetBarComponent& TopChromeComponent::getPresetBar() noexcept { return presetBar; }
juce::TextButton& TopChromeComponent::getAudioButton() noexcept { return audioButton; }

} // namespace lumen::ui
