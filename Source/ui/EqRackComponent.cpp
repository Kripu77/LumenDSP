#include "ui/EqRackComponent.h"
#include "parameters/ParameterIds.h"

namespace lumen::ui
{

EqRackComponent::EqRackComponent()
{
    titleLabel.setText("GRAPHIC TONE", juce::dontSendNotification);
    titleLabel.setFont(design::hardwareBrandFont());
    titleLabel.setColour(juce::Label::textColourId, design::metalTextPrimary());
    titleLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(titleLabel);

    bandHintLabel.setText("POST-AMP  |  PRE-CAB", juce::dontSendNotification);
    bandHintLabel.setFont(design::microFont());
    bandHintLabel.setColour(juce::Label::textColourId, design::metalTextMuted());
    bandHintLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(bandHintLabel);

    configureSlider(bassSlider, bassLabel, "BASS\n100 Hz");
    configureSlider(midSlider, midLabel, "MID\n750 Hz");
    configureSlider(trebleSlider, trebleLabel, "TREBLE\n4 kHz");

    enableButton.setColour(juce::ToggleButton::textColourId, design::metalTextSecondary());
    addAndMakeVisible(enableButton);
}

void EqRackComponent::paint(juce::Graphics& graphics)
{
    auto bounds = getLocalBounds().toFloat().reduced(14.0f, 18.0f);
    design::drawSoftShadow(graphics, bounds, 12.0f, 0.24f);

    juce::ColourGradient body(
        juce::Colour::fromRGB(54, 58, 64),
        bounds.getX(),
        bounds.getY(),
        juce::Colour::fromRGB(26, 28, 32),
        bounds.getX(),
        bounds.getBottom(),
        false);
    graphics.setGradientFill(body);
    graphics.fillRoundedRectangle(bounds, 12.0f);
    graphics.setColour(juce::Colours::white.withAlpha(0.07f));
    graphics.drawRoundedRectangle(bounds.reduced(1.0f), 11.0f, 1.0f);
    graphics.setColour(juce::Colours::black.withAlpha(0.5f));
    graphics.drawRoundedRectangle(bounds, 12.0f, 1.3f);

    auto face = bounds.reduced(18.0f, 16.0f);
    graphics.setColour(juce::Colour::fromRGB(16, 17, 20).withAlpha(0.88f));
    graphics.fillRoundedRectangle(face, 8.0f);
    graphics.setColour(design::metalBorder().withAlpha(0.55f));
    graphics.drawRoundedRectangle(face, 8.0f, 1.0f);

    for (int footIndex = 0; footIndex < 2; ++footIndex)
    {
        const float x = footIndex == 0 ? bounds.getX() + 28.0f : bounds.getRight() - 48.0f;
        graphics.setColour(design::metalDeep());
        graphics.fillRoundedRectangle(x, bounds.getBottom() - 10.0f, 20.0f, 10.0f, 3.0f);
    }

    graphics.setColour(design::ledWarm().withAlpha(0.3f));
    graphics.fillEllipse(bounds.getX() + 20.0f, bounds.getBottom() - 40.0f, 18.0f, 18.0f);
    graphics.setColour(design::ledWarm());
    graphics.fillEllipse(bounds.getX() + 24.0f, bounds.getBottom() - 36.0f, 10.0f, 10.0f);

    const bool powered = enableButton.getToggleState();
    graphics.setColour(powered ? design::ledWarm().withAlpha(0.3f) : juce::Colours::transparentBlack);
    graphics.fillEllipse(bounds.getRight() - 42.0f, bounds.getBottom() - 40.0f, 18.0f, 18.0f);
    graphics.setColour(powered ? design::ledWarm() : design::metalTextMuted());
    graphics.fillEllipse(bounds.getRight() - 38.0f, bounds.getBottom() - 36.0f, 10.0f, 10.0f);
}

void EqRackComponent::resized()
{
    auto bounds = getLocalBounds().reduced(36, 34);
    titleLabel.setBounds(bounds.removeFromTop(26));
    bandHintLabel.setBounds(bounds.removeFromTop(18));
    bounds.removeFromTop(10);

    auto power = bounds.removeFromBottom(28);
    enableButton.setBounds(power.withSizeKeepingCentre(120, 24));
    bounds.removeFromBottom(10);

    const int columnWidth = bounds.getWidth() / 3;
    auto bassCol = bounds.removeFromLeft(columnWidth);
    auto midCol = bounds.removeFromLeft(columnWidth);
    auto trebleCol = bounds;

    bassLabel.setBounds(bassCol.removeFromBottom(40));
    midLabel.setBounds(midCol.removeFromBottom(40));
    trebleLabel.setBounds(trebleCol.removeFromBottom(40));

    bassSlider.setBounds(bassCol.reduced(36, 6));
    midSlider.setBounds(midCol.reduced(36, 6));
    trebleSlider.setBounds(trebleCol.reduced(36, 6));
}

juce::Slider& EqRackComponent::getBassSlider() noexcept { return bassSlider; }
juce::Slider& EqRackComponent::getMidSlider() noexcept { return midSlider; }
juce::Slider& EqRackComponent::getTrebleSlider() noexcept { return trebleSlider; }
juce::ToggleButton& EqRackComponent::getEnableButton() noexcept { return enableButton; }

void EqRackComponent::configureSlider(juce::Slider& slider, juce::Label& label, const juce::String& text)
{
    slider.setSliderStyle(juce::Slider::LinearVertical);
    slider.setTextBoxStyle(juce::Slider::NoTextBox, true, 0, 0);
    slider.setRange(
        parameters::ranges::eqGainMinimumDb,
        parameters::ranges::eqGainMaximumDb,
        parameters::ranges::eqGainStepDb);
    slider.setValue(parameters::ranges::eqGainDefaultDb, juce::dontSendNotification);
    slider.setSliderSnapsToMousePosition(false);
    addAndMakeVisible(slider);

    label.setText(text, juce::dontSendNotification);
    label.setFont(design::microFont());
    label.setColour(juce::Label::textColourId, design::metalTextSecondary());
    label.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(label);
}

} // namespace lumen::ui
