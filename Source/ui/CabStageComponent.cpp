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

    statusLabel.setFont(design::sectionFont());
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
    auto bounds = getLocalBounds().toFloat().reduced(16.0f, 20.0f);
    design::drawSoftShadow(graphics, bounds, static_cast<float>(design::panelCornerRadiusPixels), 0.24f);

    juce::ColourGradient shell(
        juce::Colour::fromRGB(48, 42, 36),
        bounds.getX(),
        bounds.getY(),
        juce::Colour::fromRGB(24, 20, 18),
        bounds.getX(),
        bounds.getBottom(),
        false);
    graphics.setGradientFill(shell);
    graphics.fillRoundedRectangle(bounds, static_cast<float>(design::panelCornerRadiusPixels));
    graphics.setColour(juce::Colours::white.withAlpha(0.06f));
    graphics.drawRoundedRectangle(bounds.reduced(1.0f), static_cast<float>(design::panelCornerRadiusPixels) - 1.0f, 1.0f);
    graphics.setColour(juce::Colours::black.withAlpha(0.5f));
    graphics.drawRoundedRectangle(bounds, static_cast<float>(design::panelCornerRadiusPixels), 1.3f);

    auto cabBody = bounds.reduced(40.0f, 48.0f).withTrimmedBottom(100.0f);
    juce::ColourGradient cab(
        juce::Colour::fromRGB(34, 30, 28),
        cabBody.getX(),
        cabBody.getY(),
        juce::Colour::fromRGB(16, 14, 13),
        cabBody.getX(),
        cabBody.getBottom(),
        false);
    graphics.setGradientFill(cab);
    graphics.fillRoundedRectangle(cabBody, 10.0f);
    graphics.setColour(design::metalBorder().withAlpha(0.45f));
    graphics.drawRoundedRectangle(cabBody, 10.0f, 1.3f);

    for (float y = cabBody.getY() + 10.0f; y < cabBody.getBottom() - 10.0f; y += 7.0f)
    {
        graphics.setColour(juce::Colours::black.withAlpha(0.18f));
        graphics.drawLine(cabBody.getX() + 10.0f, y, cabBody.getRight() - 10.0f, y, 1.0f);
    }

    auto speaker = cabBody.withSizeKeepingCentre(cabBody.getHeight() * 0.62f, cabBody.getHeight() * 0.62f);
    graphics.setColour(juce::Colours::black.withAlpha(0.35f));
    graphics.fillEllipse(speaker.expanded(4.0f));

    juce::ColourGradient cone(
        juce::Colour::fromRGB(72, 76, 82),
        speaker.getCentreX() - 10.0f,
        speaker.getCentreY() - 12.0f,
        juce::Colour::fromRGB(28, 30, 34),
        speaker.getCentreX() + 8.0f,
        speaker.getCentreY() + 14.0f,
        false);
    graphics.setGradientFill(cone);
    graphics.fillEllipse(speaker);
    graphics.setColour(design::metalBorder());
    graphics.drawEllipse(speaker, 2.0f);
    graphics.drawEllipse(speaker.withSizeKeepingCentre(speaker.getWidth() * 0.62f, speaker.getHeight() * 0.62f), 1.5f);
    graphics.drawEllipse(speaker.withSizeKeepingCentre(speaker.getWidth() * 0.34f, speaker.getHeight() * 0.34f), 1.3f);
    graphics.setColour(design::metalDeep());
    graphics.fillEllipse(speaker.withSizeKeepingCentre(speaker.getWidth() * 0.14f, speaker.getHeight() * 0.14f));
    graphics.setColour(design::metalBorder().withAlpha(0.7f));
    graphics.drawEllipse(speaker.withSizeKeepingCentre(speaker.getWidth() * 0.14f, speaker.getHeight() * 0.14f), 1.0f);

    auto badge = juce::Rectangle<float>(cabBody.getCentreX() - 42.0f, cabBody.getBottom() - 28.0f, 84.0f, 16.0f);
    graphics.setColour(design::metalDeep().withAlpha(0.8f));
    graphics.fillRoundedRectangle(badge, 3.0f);
    graphics.setColour(design::metalTextMuted());
    graphics.setFont(design::microFont());
    graphics.drawText("LUMEN 1x12", badge, juce::Justification::centred);

    const bool engaged = enableButton.getToggleState();
    graphics.setColour(engaged ? design::ledWarm().withAlpha(0.3f) : juce::Colours::transparentBlack);
    graphics.fillEllipse(bounds.getRight() - 44.0f, bounds.getY() + 20.0f, 18.0f, 18.0f);
    graphics.setColour(engaged ? design::ledWarm() : design::metalTextMuted());
    graphics.fillEllipse(bounds.getRight() - 40.0f, bounds.getY() + 24.0f, 10.0f, 10.0f);
}

void CabStageComponent::resized()
{
    auto bounds = getLocalBounds().reduced(40, 36);
    titleLabel.setBounds(bounds.removeFromTop(26));
    statusLabel.setBounds(bounds.removeFromTop(20));
    bounds.removeFromTop(8);

    bounds.removeFromTop(static_cast<int>(bounds.getHeight() * 0.52f));
    bounds.removeFromTop(8);

    enableButton.setBounds(bounds.removeFromTop(28).withSizeKeepingCentre(150, 24));
    bounds.removeFromTop(10);
    irSlot.setBounds(bounds.removeFromTop(80).reduced(8, 0));
    footerLabel.setBounds(bounds.removeFromBottom(18));
}

FileSlotComponent& CabStageComponent::getIrSlot() noexcept { return irSlot; }
juce::ToggleButton& CabStageComponent::getEnableButton() noexcept { return enableButton; }

void CabStageComponent::setIrStatus(const juce::String& irName, bool isLoaded)
{
    if (isLoaded)
    {
        statusLabel.setText(irName.toUpperCase(), juce::dontSendNotification);
        statusLabel.setColour(juce::Label::textColourId, design::ledWarm());
    }
    else
    {
        statusLabel.setText("NO IR LOADED", juce::dontSendNotification);
        statusLabel.setColour(juce::Label::textColourId, design::metalTextMuted());
    }
}

} // namespace lumen::ui
