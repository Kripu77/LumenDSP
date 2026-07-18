#include "ui/AmpFaceplateComponent.h"

namespace lumen::ui
{

AmpFaceplateComponent::AmpFaceplateComponent()
{
    chassisLabel.setText("Amp", juce::dontSendNotification);
    chassisLabel.setFont(design::titleFont());
    chassisLabel.setColour(juce::Label::textColourId, design::textPrimary());
    chassisLabel.setJustificationType(juce::Justification::centredLeft);
    addAndMakeVisible(chassisLabel);

    modelStatusLabel.setFont(design::sectionFont());
    modelStatusLabel.setColour(juce::Label::textColourId, design::textSecondary());
    modelStatusLabel.setJustificationType(juce::Justification::centredLeft);
    addAndMakeVisible(modelStatusLabel);

    brandFooterLabel.setText("Neural Amp Modeler Core", juce::dontSendNotification);
    brandFooterLabel.setFont(design::microFont());
    brandFooterLabel.setColour(juce::Label::textColourId, design::textMuted());
    brandFooterLabel.setJustificationType(juce::Justification::centredRight);
    addAndMakeVisible(brandFooterLabel);

    addAndMakeVisible(modelSlot);
    addAndMakeVisible(bassKnob);
    addAndMakeVisible(midKnob);
    addAndMakeVisible(trebleKnob);
    addAndMakeVisible(eqEnableButton);

    modelSlot.setDarkTheme(true);
    eqEnableButton.setColour(juce::ToggleButton::textColourId, design::textSecondary());
}

void AmpFaceplateComponent::paint(juce::Graphics& graphics)
{
    auto bounds = getLocalBounds().toFloat();
    design::drawCard(graphics, bounds, 12.0f);

    auto accentBar = bounds.removeFromTop(3.0f).reduced(12.0f, 0.0f);
    graphics.setColour(design::accent());
    graphics.fillRoundedRectangle(accentBar, 1.5f);

    auto knobsBay = getLocalBounds().toFloat().reduced(20.0f).withTrimmedTop(150.0f).withTrimmedBottom(48.0f);
    graphics.setColour(design::bgTertiary());
    graphics.fillRoundedRectangle(knobsBay, 10.0f);
    graphics.setColour(design::borderLight());
    graphics.drawRoundedRectangle(knobsBay, 10.0f, 1.0f);

    juce::ColourGradient glow(
        design::accent().withAlpha(0.12f),
        knobsBay.getCentreX(),
        knobsBay.getY(),
        juce::Colours::transparentBlack,
        knobsBay.getCentreX(),
        knobsBay.getBottom(),
        false);
    graphics.setGradientFill(glow);
    graphics.fillRoundedRectangle(knobsBay, 10.0f);
}

void AmpFaceplateComponent::resized()
{
    auto bounds = getLocalBounds().reduced(20, 16);
    auto header = bounds.removeFromTop(28);
    chassisLabel.setBounds(header.removeFromLeft(80));
    brandFooterLabel.setBounds(header.removeFromRight(180));
    modelStatusLabel.setBounds(header);

    bounds.removeFromTop(8);
    modelSlot.setBounds(bounds.removeFromTop(76));
    bounds.removeFromTop(12);

    auto knobs = bounds.removeFromTop(140);
    const int knobWidth = knobs.getWidth() / 3;
    bassKnob.setBounds(knobs.removeFromLeft(knobWidth).reduced(18, 8));
    midKnob.setBounds(knobs.removeFromLeft(knobWidth).reduced(18, 8));
    trebleKnob.setBounds(knobs.reduced(18, 8));

    bounds.removeFromTop(8);
    eqEnableButton.setBounds(bounds.removeFromTop(26).withSizeKeepingCentre(160, 24));
}

FileSlotComponent& AmpFaceplateComponent::getModelSlot() noexcept { return modelSlot; }
KnobComponent& AmpFaceplateComponent::getBassKnob() noexcept { return bassKnob; }
KnobComponent& AmpFaceplateComponent::getMidKnob() noexcept { return midKnob; }
KnobComponent& AmpFaceplateComponent::getTrebleKnob() noexcept { return trebleKnob; }
juce::ToggleButton& AmpFaceplateComponent::getEqEnableButton() noexcept { return eqEnableButton; }

void AmpFaceplateComponent::setModelStatus(const juce::String& modelName, bool isLoaded, bool isLoading)
{
    if (isLoading)
    {
        modelStatusLabel.setText("Loading model...", juce::dontSendNotification);
        modelStatusLabel.setColour(juce::Label::textColourId, design::accent());
    }
    else if (isLoaded)
    {
        modelStatusLabel.setText(modelName, juce::dontSendNotification);
        modelStatusLabel.setColour(juce::Label::textColourId, design::success());
    }
    else
    {
        modelStatusLabel.setText("No model loaded", juce::dontSendNotification);
        modelStatusLabel.setColour(juce::Label::textColourId, design::textMuted());
    }
}

void AmpFaceplateComponent::paintTubeGlow(juce::Graphics&, juce::Rectangle<float>) const
{
}

} // namespace lumen::ui
