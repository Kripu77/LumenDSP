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
    design::drawSoftShadow(graphics, bounds, static_cast<float>(design::panelCornerRadiusPixels), 0.10f);

    juce::ColourGradient chrome(
        design::chromeSurfaceRaised(),
        bounds.getX(),
        bounds.getY(),
        design::chromeSurface(),
        bounds.getX(),
        bounds.getBottom(),
        false);
    graphics.setGradientFill(chrome);
    graphics.fillRoundedRectangle(bounds.reduced(0.5f), static_cast<float>(design::panelCornerRadiusPixels));
    graphics.setColour(design::chromeBorder());
    graphics.drawRoundedRectangle(bounds.reduced(0.5f), static_cast<float>(design::panelCornerRadiusPixels), 1.0f);
}

void TopChromeComponent::resized()
{
    auto bounds = getLocalBounds().reduced(design::spacingTwoUnitsPixels, design::spacingUnitPixels);

    auto brandArea = bounds.removeFromLeft(88);
    brandLabel.setBounds(brandArea.removeFromTop(26).withTrimmedTop(4));
    productLabel.setBounds(brandArea.removeFromTop(22));

    bounds.removeFromLeft(design::spacingUnitPixels);

    auto inputCluster = bounds.removeFromLeft(118);
    inputMeter.setBounds(inputCluster.removeFromLeft(16).reduced(1, 8));
    inputKnob.setBounds(inputCluster.reduced(2, 0));

    bounds.removeFromLeft(design::spacingHalfUnitPixels);

    auto gateCluster = bounds.removeFromLeft(118);
    gateEnableButton.setBounds(gateCluster.removeFromTop(22).withSizeKeepingCentre(92, 20));
    gateKnob.setBounds(gateCluster.reduced(2, 0));

    auto right = bounds.removeFromRight(200);
    audioButton.setBounds(right.removeFromTop(30).removeFromRight(84).reduced(0, 1));
    right.removeFromTop(2);
    auto outputCluster = right;
    outputMeter.setBounds(outputCluster.removeFromRight(16).reduced(1, 8));
    outputKnob.setBounds(outputCluster.reduced(2, 0));

    bounds.reduce(design::spacingUnitPixels, 4);
    presetBar.setBounds(bounds.withSizeKeepingCentre(juce::jmax(280, bounds.getWidth()), 40));
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
