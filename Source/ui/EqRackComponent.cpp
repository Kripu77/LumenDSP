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

    bandHintLabel.setText("POST-AMP  ·  PRE-CAB", juce::dontSendNotification);
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
    auto bounds = getLocalBounds().toFloat().reduced(18.0f, 24.0f);
    design::drawMetalPanel(graphics, bounds, 12.0f);

    auto face = bounds.reduced(20.0f, 18.0f);
    graphics.setColour(design::metalDeep().withAlpha(0.65f));
    graphics.fillRoundedRectangle(face, 8.0f);

    graphics.setColour(design::ledWarm());
    graphics.fillEllipse(bounds.getX() + 22.0f, bounds.getBottom() - 36.0f, 12.0f, 12.0f);
    graphics.setColour(design::ledWarm().withAlpha(0.3f));
    graphics.fillEllipse(bounds.getX() + 18.0f, bounds.getBottom() - 40.0f, 20.0f, 20.0f);

    graphics.setColour(enableButton.getToggleState() ? design::ledWarm() : design::metalTextMuted());
    graphics.fillEllipse(bounds.getRight() - 36.0f, bounds.getBottom() - 36.0f, 12.0f, 12.0f);
}

void EqRackComponent::resized()
{
    auto bounds = getLocalBounds().reduced(40, 40);
    titleLabel.setBounds(bounds.removeFromTop(28));
    bandHintLabel.setBounds(bounds.removeFromTop(18));
    bounds.removeFromTop(8);

    auto power = bounds.removeFromBottom(28);
    enableButton.setBounds(power.withSizeKeepingCentre(120, 24));
    bounds.removeFromBottom(8);

    const int columnWidth = bounds.getWidth() / 3;
    auto bassCol = bounds.removeFromLeft(columnWidth);
    auto midCol = bounds.removeFromLeft(columnWidth);
    auto trebleCol = bounds;

    bassLabel.setBounds(bassCol.removeFromBottom(36));
    midLabel.setBounds(midCol.removeFromBottom(36));
    trebleLabel.setBounds(trebleCol.removeFromBottom(36));

    bassSlider.setBounds(bassCol.reduced(28, 8));
    midSlider.setBounds(midCol.reduced(28, 8));
    trebleSlider.setBounds(trebleCol.reduced(28, 8));
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
    addAndMakeVisible(slider);

    label.setText(text, juce::dontSendNotification);
    label.setFont(design::microFont());
    label.setColour(juce::Label::textColourId, design::metalTextSecondary());
    label.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(label);
}

} // namespace lumen::ui
