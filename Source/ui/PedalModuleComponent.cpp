#include "ui/PedalModuleComponent.h"

namespace lumen::ui
{

PedalModuleComponent::PedalModuleComponent(
    const juce::String& titleText,
    const juce::String& subtitleText,
    Finish finish)
    : title(titleText)
    , subtitle(subtitleText)
    , pedalFinish(finish)
    , primaryKnob("THRESH", "dB", KnobComponent::Style::metal)
{
    titleLabel.setText(title, juce::dontSendNotification);
    titleLabel.setFont(design::hardwareBrandFont());
    titleLabel.setJustificationType(juce::Justification::centred);
    titleLabel.setColour(juce::Label::textColourId, design::metalTextPrimary());
    addAndMakeVisible(titleLabel);

    subtitleLabel.setText(subtitle, juce::dontSendNotification);
    subtitleLabel.setFont(design::microFont());
    subtitleLabel.setJustificationType(juce::Justification::centred);
    subtitleLabel.setColour(juce::Label::textColourId, design::metalTextMuted());
    addAndMakeVisible(subtitleLabel);

    enableButton.setColour(juce::ToggleButton::textColourId, design::metalTextSecondary());
    addAndMakeVisible(primaryKnob);
    addAndMakeVisible(enableButton);
}

void PedalModuleComponent::paint(juce::Graphics& graphics)
{
    auto bounds = getLocalBounds().toFloat().reduced(4.0f);
    design::drawSoftShadow(graphics, bounds, static_cast<float>(design::pedalCornerRadiusPixels), 0.28f);

    juce::ColourGradient body(
        finishColour().brighter(0.12f),
        bounds.getX(),
        bounds.getY(),
        finishColour().darker(0.22f),
        bounds.getX(),
        bounds.getBottom(),
        false);
    graphics.setGradientFill(body);
    graphics.fillRoundedRectangle(bounds, static_cast<float>(design::pedalCornerRadiusPixels));

    auto lip = bounds.removeFromTop(10.0f);
    graphics.setColour(juce::Colours::white.withAlpha(0.10f));
    graphics.fillRoundedRectangle(lip.withTrimmedBottom(2.0f), 8.0f);

    bounds = getLocalBounds().toFloat().reduced(4.0f);

    for (const auto& screwCentre :
         {juce::Point<float>(bounds.getX() + 16.0f, bounds.getY() + 16.0f),
          juce::Point<float>(bounds.getRight() - 16.0f, bounds.getY() + 16.0f),
          juce::Point<float>(bounds.getX() + 16.0f, bounds.getBottom() - 16.0f),
          juce::Point<float>(bounds.getRight() - 16.0f, bounds.getBottom() - 16.0f)})
    {
        graphics.setColour(design::metalDeep().withAlpha(0.65f));
        graphics.fillEllipse(screwCentre.x - 4.0f, screwCentre.y - 4.0f, 8.0f, 8.0f);
        graphics.setColour(design::metalBorder());
        graphics.drawEllipse(screwCentre.x - 4.0f, screwCentre.y - 4.0f, 8.0f, 8.0f, 1.0f);
        graphics.drawLine(screwCentre.x - 2.2f, screwCentre.y, screwCentre.x + 2.2f, screwCentre.y, 1.0f);
    }

    graphics.setColour(juce::Colours::white.withAlpha(0.10f));
    graphics.drawRoundedRectangle(bounds.reduced(1.0f), static_cast<float>(design::pedalCornerRadiusPixels) - 1.0f, 1.2f);
    graphics.setColour(juce::Colours::black.withAlpha(0.5f));
    graphics.drawRoundedRectangle(bounds, static_cast<float>(design::pedalCornerRadiusPixels), 1.4f);

    auto led = juce::Rectangle<float>(bounds.getCentreX() - 8.0f, bounds.getY() + 22.0f, 16.0f, 16.0f);
    if (engaged)
    {
        graphics.setColour(design::ledCool().withAlpha(0.28f));
        graphics.fillEllipse(led.expanded(8.0f));
    }
    graphics.setColour(engaged ? design::ledCool() : design::metalTextMuted().darker(0.2f));
    graphics.fillEllipse(led);
    graphics.setColour(juce::Colours::white.withAlpha(engaged ? 0.45f : 0.12f));
    graphics.fillEllipse(led.reduced(4.5f).translated(-1.0f, -1.5f));

    auto jack = juce::Rectangle<float>(bounds.getX() + 18.0f, bounds.getCentreY() - 7.0f, 14.0f, 14.0f);
    graphics.setColour(design::metalDeep());
    graphics.fillEllipse(jack);
    graphics.setColour(design::metalBorder());
    graphics.drawEllipse(jack, 1.0f);

    auto footswitch = juce::Rectangle<float>(bounds.getCentreX() - 22.0f, bounds.getBottom() - 62.0f, 44.0f, 44.0f);
    graphics.setColour(juce::Colours::black.withAlpha(0.35f));
    graphics.fillEllipse(footswitch.translated(0.0f, 3.0f));
    juce::ColourGradient fs(
        design::metalRaised().brighter(0.08f),
        footswitch.getCentreX(),
        footswitch.getY(),
        design::metalDeep(),
        footswitch.getCentreX(),
        footswitch.getBottom(),
        false);
    graphics.setGradientFill(fs);
    graphics.fillEllipse(footswitch);
    graphics.setColour(design::metalBorder());
    graphics.drawEllipse(footswitch, 1.6f);
    graphics.setColour(design::metalDeep().withAlpha(0.8f));
    graphics.fillEllipse(footswitch.reduced(12.0f));
}

void PedalModuleComponent::resized()
{
    auto bounds = getLocalBounds().reduced(22, 20);
    titleLabel.setBounds(bounds.removeFromTop(30));
    subtitleLabel.setBounds(bounds.removeFromTop(16));
    bounds.removeFromTop(10);
    enableButton.setBounds(bounds.removeFromTop(26).withSizeKeepingCentre(120, 24));
    bounds.removeFromBottom(68);
    primaryKnob.setBounds(bounds.reduced(20, 8));
}

KnobComponent& PedalModuleComponent::getPrimaryKnob() noexcept
{
    return primaryKnob;
}

juce::ToggleButton& PedalModuleComponent::getEnableButton() noexcept
{
    return enableButton;
}

void PedalModuleComponent::setEngaged(bool isEngaged)
{
    engaged = isEngaged;
    repaint();
}

juce::Colour PedalModuleComponent::finishColour() const
{
    switch (pedalFinish)
    {
        case Finish::olive:
            return juce::Colour::fromRGB(78, 92, 74);
        case Finish::charcoal:
            return juce::Colour::fromRGB(40, 44, 50);
        case Finish::steel:
        default:
            return juce::Colour::fromRGB(118, 124, 132);
    }
}

} // namespace lumen::ui
