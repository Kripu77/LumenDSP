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
    productLabel.setColour(juce::Label::textColourId, design::chromeTextPrimary());
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
    graphics.setColour(design::chromeSurface().withAlpha(0.92f));
    graphics.fillRoundedRectangle(bounds.reduced(0.5f), static_cast<float>(design::panelCornerRadiusPixels));
    graphics.setColour(design::chromeBorder());
    graphics.drawRoundedRectangle(bounds.reduced(0.5f), static_cast<float>(design::panelCornerRadiusPixels), 1.0f);

    graphics.setColour(design::chromeBorder().withAlpha(0.65f));
    const float dividerX = static_cast<float>(getWidth()) * 0.18f;
    graphics.drawLine(dividerX, 18.0f, dividerX, static_cast<float>(getHeight()) - 18.0f, 1.0f);
}

void TopChromeComponent::resized()
{
    auto bounds = getLocalBounds().reduced(design::spacingTwoUnitsPixels, design::spacingUnitPixels + 2);

    auto brandArea = bounds.removeFromLeft(120);
    brandLabel.setBounds(brandArea.removeFromTop(28));
    productLabel.setBounds(brandArea.removeFromTop(24));

    bounds.removeFromLeft(design::spacingTwoUnitsPixels);
    auto inputCluster = bounds.removeFromLeft(150);
    inputMeter.setBounds(inputCluster.removeFromLeft(design::chromeMeterWidthPixels + 4));
    inputKnob.setBounds(inputCluster);

    bounds.removeFromLeft(design::spacingUnitPixels);
    auto gateCluster = bounds.removeFromLeft(150);
    auto gateToggle = gateCluster.removeFromTop(24);
    gateEnableButton.setBounds(gateToggle.withSizeKeepingCentre(100, 22));
    gateKnob.setBounds(gateCluster);

    auto right = bounds.removeFromRight(220);
    audioButton.setBounds(right.removeFromTop(32).removeFromRight(88));
    right.removeFromTop(design::spacingHalfUnitPixels);
    auto outputCluster = right;
    outputKnob.setBounds(outputCluster.removeFromLeft(outputCluster.getWidth() - design::chromeMeterWidthPixels - 6));
    outputMeter.setBounds(outputCluster);

    bounds.reduce(design::spacingTwoUnitsPixels, design::spacingUnitPixels);
    presetBar.setBounds(bounds.withSizeKeepingCentre(bounds.getWidth(), 44));
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
