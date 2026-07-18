#include "ui/EqRackComponent.h"
#include "parameters/ParameterIds.h"

namespace lumen::ui
{

EqRackComponent::EqRackComponent()
{
    titleLabel.setText("EQ", juce::dontSendNotification);
    titleLabel.setFont(design::titleFont());
    titleLabel.setColour(juce::Label::textColourId, design::textPrimary());
    titleLabel.setJustificationType(juce::Justification::centredLeft);
    addAndMakeVisible(titleLabel);

    bandHintLabel.setText("Post-amp  |  Pre-cab", juce::dontSendNotification);
    bandHintLabel.setFont(design::microFont());
    bandHintLabel.setColour(juce::Label::textColourId, design::textMuted());
    bandHintLabel.setJustificationType(juce::Justification::centredRight);
    addAndMakeVisible(bandHintLabel);

    configureSlider(bassSlider, bassLabel, "BASS\n100 Hz");
    configureSlider(midSlider, midLabel, "MID\n750 Hz");
    configureSlider(trebleSlider, trebleLabel, "TREBLE\n4 kHz");

    enableButton.setColour(juce::ToggleButton::textColourId, design::textSecondary());
    addAndMakeVisible(enableButton);
}

void EqRackComponent::paint(juce::Graphics& graphics)
{
    design::drawCard(graphics, getLocalBounds().toFloat(), 12.0f);

    auto bay = getLocalBounds().toFloat().reduced(18.0f, 56.0f).withTrimmedBottom(36.0f);
    graphics.setColour(design::bgTertiary());
    graphics.fillRoundedRectangle(bay, 10.0f);
    graphics.setColour(design::borderLight());
    graphics.drawRoundedRectangle(bay, 10.0f, 1.0f);
}

void EqRackComponent::resized()
{
    auto bounds = getLocalBounds().reduced(20, 16);
    auto header = bounds.removeFromTop(24);
    titleLabel.setBounds(header.removeFromLeft(60));
    bandHintLabel.setBounds(header);

    bounds.removeFromTop(10);
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

    bassSlider.setBounds(bassCol.reduced(40, 4));
    midSlider.setBounds(midCol.reduced(40, 4));
    trebleSlider.setBounds(trebleCol.reduced(40, 4));
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
    label.setColour(juce::Label::textColourId, design::textMuted());
    label.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(label);
}

} // namespace lumen::ui
