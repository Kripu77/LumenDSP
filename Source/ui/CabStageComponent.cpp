#include "ui/CabStageComponent.h"

namespace lumen::ui
{

CabStageComponent::CabStageComponent()
{
    titleLabel.setText("Cabinet", juce::dontSendNotification);
    titleLabel.setFont(design::titleFont());
    titleLabel.setColour(juce::Label::textColourId, design::textPrimary());
    titleLabel.setJustificationType(juce::Justification::centredLeft);
    addAndMakeVisible(titleLabel);

    statusLabel.setFont(design::sectionFont());
    statusLabel.setColour(juce::Label::textColourId, design::textSecondary());
    statusLabel.setJustificationType(juce::Justification::centredLeft);
    addAndMakeVisible(statusLabel);

    footerLabel.setText("Impulse response", juce::dontSendNotification);
    footerLabel.setFont(design::microFont());
    footerLabel.setColour(juce::Label::textColourId, design::textMuted());
    footerLabel.setJustificationType(juce::Justification::centredRight);
    addAndMakeVisible(footerLabel);

    irSlot.setDarkTheme(true);
    enableButton.setColour(juce::ToggleButton::textColourId, design::textSecondary());
    addAndMakeVisible(irSlot);
    addAndMakeVisible(enableButton);
}

void CabStageComponent::paint(juce::Graphics& graphics)
{
    auto bounds = getLocalBounds().toFloat();
    design::drawCard(graphics, bounds, 12.0f);

    auto cab = bounds.reduced(28.0f, 70.0f).withTrimmedBottom(90.0f);
    graphics.setColour(design::bgTertiary());
    graphics.fillRoundedRectangle(cab, 10.0f);
    graphics.setColour(design::borderLight());
    graphics.drawRoundedRectangle(cab, 10.0f, 1.0f);

    auto speaker = cab.withSizeKeepingCentre(cab.getHeight() * 0.7f, cab.getHeight() * 0.7f);
    juce::ColourGradient cone(
        design::knobBodyFrom(),
        speaker.getCentreX() - 8.0f,
        speaker.getCentreY() - 10.0f,
        design::knobBodyTo(),
        speaker.getCentreX() + 6.0f,
        speaker.getCentreY() + 12.0f,
        false);
    graphics.setGradientFill(cone);
    graphics.fillEllipse(speaker);
    graphics.setColour(design::borderStrong());
    graphics.drawEllipse(speaker, 2.0f);
    graphics.drawEllipse(speaker.withSizeKeepingCentre(speaker.getWidth() * 0.55f, speaker.getHeight() * 0.55f), 1.4f);
    graphics.setColour(design::bgPrimary());
    graphics.fillEllipse(speaker.withSizeKeepingCentre(speaker.getWidth() * 0.16f, speaker.getHeight() * 0.16f));

    const bool on = enableButton.getToggleState();
    graphics.setColour(on ? design::success() : design::textMuted());
    graphics.fillEllipse(bounds.getRight() - 34.0f, bounds.getY() + 18.0f, 10.0f, 10.0f);
}

void CabStageComponent::resized()
{
    auto bounds = getLocalBounds().reduced(20, 16);
    auto header = bounds.removeFromTop(24);
    titleLabel.setBounds(header.removeFromLeft(100));
    footerLabel.setBounds(header.removeFromRight(140));
    statusLabel.setBounds(header);

    bounds.removeFromTop(8);
    bounds.removeFromTop(static_cast<int>(bounds.getHeight() * 0.48f));
    bounds.removeFromTop(8);

    enableButton.setBounds(bounds.removeFromTop(26).withSizeKeepingCentre(140, 24));
    bounds.removeFromTop(8);
    irSlot.setBounds(bounds.removeFromTop(78));
}

FileSlotComponent& CabStageComponent::getIrSlot() noexcept { return irSlot; }
juce::ToggleButton& CabStageComponent::getEnableButton() noexcept { return enableButton; }

void CabStageComponent::setIrStatus(const juce::String& irName, bool isLoaded)
{
    if (isLoaded)
    {
        statusLabel.setText(irName, juce::dontSendNotification);
        statusLabel.setColour(juce::Label::textColourId, design::success());
    }
    else
    {
        statusLabel.setText("No IR loaded", juce::dontSendNotification);
        statusLabel.setColour(juce::Label::textColourId, design::textMuted());
    }
}

} // namespace lumen::ui
