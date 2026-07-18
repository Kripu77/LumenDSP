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
    auto bounds = getLocalBounds().toFloat().reduced(6.0f);
    design::drawSoftShadow(graphics, bounds, static_cast<float>(design::pedalCornerRadiusPixels), 0.25f);

    juce::ColourGradient body(
        finishColour().brighter(0.08f),
        bounds.getX(),
        bounds.getY(),
        finishColour().darker(0.18f),
        bounds.getX(),
        bounds.getBottom(),
        false);
    graphics.setGradientFill(body);
    graphics.fillRoundedRectangle(bounds, static_cast<float>(design::pedalCornerRadiusPixels));

    graphics.setColour(juce::Colours::white.withAlpha(0.08f));
    graphics.drawRoundedRectangle(bounds.reduced(1.0f), static_cast<float>(design::pedalCornerRadiusPixels) - 1.0f, 1.0f);
    graphics.setColour(juce::Colours::black.withAlpha(0.45f));
    graphics.drawRoundedRectangle(bounds, static_cast<float>(design::pedalCornerRadiusPixels), 1.3f);

    auto led = juce::Rectangle<float>(bounds.getCentreX() - 7.0f, bounds.getY() + 18.0f, 14.0f, 14.0f);
    graphics.setColour(engaged ? design::ledCool().withAlpha(0.35f) : juce::Colours::transparentBlack);
    graphics.fillEllipse(led.expanded(6.0f));
    graphics.setColour(engaged ? design::ledCool() : design::metalTextMuted());
    graphics.fillEllipse(led);

    auto footswitch = juce::Rectangle<float>(bounds.getCentreX() - 18.0f, bounds.getBottom() - 52.0f, 36.0f, 36.0f);
    juce::ColourGradient fs(
        design::metalRaised(),
        footswitch.getCentreX(),
        footswitch.getY(),
        design::metalDeep(),
        footswitch.getCentreX(),
        footswitch.getBottom(),
        false);
    graphics.setGradientFill(fs);
    graphics.fillEllipse(footswitch);
    graphics.setColour(design::metalBorder());
    graphics.drawEllipse(footswitch, 1.4f);
}

void PedalModuleComponent::resized()
{
    auto bounds = getLocalBounds().reduced(18, 16);
    titleLabel.setBounds(bounds.removeFromTop(28));
    subtitleLabel.setBounds(bounds.removeFromTop(16));
    bounds.removeFromTop(8);
    enableButton.setBounds(bounds.removeFromTop(24).withSizeKeepingCentre(110, 22));
    bounds.removeFromBottom(56);
    primaryKnob.setBounds(bounds.reduced(12, 4));
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
            return juce::Colour::fromRGB(126, 132, 140);
    }
}

} // namespace lumen::ui
