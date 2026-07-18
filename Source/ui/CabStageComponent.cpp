#include "ui/CabStageComponent.h"

namespace lumen::ui
{

CabStageComponent::CabStageComponent()
{
    titleLabel.setText("CABINET", juce::dontSendNotification);
    titleLabel.setFont(design::hardwareBrandFont());
    titleLabel.setColour(juce::Label::textColourId, design::metalTextPrimary());
    titleLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(titleLabel);

    statusLabel.setFont(design::bodyFont());
    statusLabel.setColour(juce::Label::textColourId, design::metalTextSecondary());
    statusLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(statusLabel);

    footerLabel.setText("IMPULSE RESPONSE LOADER", juce::dontSendNotification);
    footerLabel.setFont(design::microFont());
    footerLabel.setColour(juce::Label::textColourId, design::metalTextMuted());
    footerLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(footerLabel);

    irSlot.setDarkTheme(true);
    enableButton.setColour(juce::ToggleButton::textColourId, design::metalTextSecondary());
    addAndMakeVisible(irSlot);
    addAndMakeVisible(enableButton);
}

void CabStageComponent::paint(juce::Graphics& graphics)
{
    auto bounds = getLocalBounds().toFloat().reduced(28.0f, 36.0f);
    design::drawMetalPanel(graphics, bounds, static_cast<float>(design::panelCornerRadiusPixels));

    auto cabBody = bounds.reduced(48.0f, 56.0f).withTrimmedBottom(90.0f);
    graphics.setColour(design::metalDeep());
    graphics.fillRoundedRectangle(cabBody, 10.0f);
    graphics.setColour(design::metalBorder());
    graphics.drawRoundedRectangle(cabBody, 10.0f, 1.4f);

    auto speaker = cabBody.withSizeKeepingCentre(cabBody.getHeight() * 0.72f, cabBody.getHeight() * 0.72f);
    graphics.setColour(design::metalRaised());
    graphics.fillEllipse(speaker);
    graphics.setColour(design::metalBorder());
    graphics.drawEllipse(speaker, 2.0f);
    graphics.drawEllipse(speaker.withSizeKeepingCentre(speaker.getWidth() * 0.55f, speaker.getHeight() * 0.55f), 1.6f);
    graphics.setColour(design::metalDeep());
    graphics.fillEllipse(speaker.withSizeKeepingCentre(speaker.getWidth() * 0.18f, speaker.getHeight() * 0.18f));

    graphics.setColour(enableButton.getToggleState() ? design::ledWarm() : design::metalTextMuted());
    graphics.fillEllipse(bounds.getRight() - 42.0f, bounds.getY() + 24.0f, 12.0f, 12.0f);
}

void CabStageComponent::resized()
{
    auto bounds = getLocalBounds().reduced(48, 48);
    titleLabel.setBounds(bounds.removeFromTop(28));
    statusLabel.setBounds(bounds.removeFromTop(22));
    bounds.removeFromTop(12);

    auto cabSpace = bounds.removeFromTop(static_cast<int>(bounds.getHeight() * 0.55f));
    juce::ignoreUnused(cabSpace);

    enableButton.setBounds(bounds.removeFromTop(28).withSizeKeepingCentre(140, 24));
    bounds.removeFromTop(10);
    irSlot.setBounds(bounds.removeFromTop(84).reduced(20, 0));
    footerLabel.setBounds(bounds.removeFromBottom(20));
}

FileSlotComponent& CabStageComponent::getIrSlot() noexcept { return irSlot; }
juce::ToggleButton& CabStageComponent::getEnableButton() noexcept { return enableButton; }

void CabStageComponent::setIrStatus(const juce::String& irName, bool isLoaded)
{
    if (isLoaded)
        statusLabel.setText(irName.toUpperCase(), juce::dontSendNotification);
    else
        statusLabel.setText("NO IR LOADED", juce::dontSendNotification);
}

} // namespace lumen::ui
