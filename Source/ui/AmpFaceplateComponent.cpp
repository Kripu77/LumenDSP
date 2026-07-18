#include "ui/AmpFaceplateComponent.h"

namespace lumen::ui
{

AmpFaceplateComponent::AmpFaceplateComponent()
{
    chassisLabel.setText("LUMEN HEAD", juce::dontSendNotification);
    chassisLabel.setFont(design::hardwareBrandFont());
    chassisLabel.setColour(juce::Label::textColourId, design::metalTextPrimary());
    chassisLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(chassisLabel);

    modelStatusLabel.setFont(design::microFont());
    modelStatusLabel.setColour(juce::Label::textColourId, design::metalTextSecondary());
    modelStatusLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(modelStatusLabel);

    brandFooterLabel.setText("OPEN SOURCE  ·  NEURAL AMP MODELER", juce::dontSendNotification);
    brandFooterLabel.setFont(design::microFont());
    brandFooterLabel.setColour(juce::Label::textColourId, design::metalTextMuted());
    brandFooterLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(brandFooterLabel);

    addAndMakeVisible(modelSlot);
    addAndMakeVisible(bassKnob);
    addAndMakeVisible(midKnob);
    addAndMakeVisible(trebleKnob);
    addAndMakeVisible(eqEnableButton);

    modelSlot.setDarkTheme(true);
    eqEnableButton.setColour(juce::ToggleButton::textColourId, design::metalTextSecondary());
}

void AmpFaceplateComponent::paint(juce::Graphics& graphics)
{
    auto bounds = getLocalBounds().toFloat().reduced(8.0f);
    design::drawMetalPanel(graphics, bounds, static_cast<float>(design::panelCornerRadiusPixels));

    auto grille = bounds.reduced(28.0f, 24.0f);
    grille = grille.removeFromTop(grille.getHeight() * 0.42f);
    paintTubeGlow(graphics, grille);

    graphics.setColour(design::metalDeep().withAlpha(0.92f));
    graphics.fillRoundedRectangle(grille, 8.0f);
    graphics.setColour(design::metalBorder());
    graphics.drawRoundedRectangle(grille, 8.0f, 1.2f);

    const int slotCount = 18;
    for (int slotIndex = 0; slotIndex < slotCount; ++slotIndex)
    {
        const float x = grille.getX() + 10.0f + static_cast<float>(slotIndex) * ((grille.getWidth() - 20.0f) / static_cast<float>(slotCount - 1));
        graphics.setColour(design::metalDeep().darker(0.25f));
        graphics.fillRect(x - 1.2f, grille.getY() + 8.0f, 2.4f, grille.getHeight() - 16.0f);
        graphics.setColour(design::metalBorder().withAlpha(0.35f));
        graphics.drawLine(x, grille.getY() + 8.0f, x, grille.getBottom() - 8.0f, 1.0f);
    }

    for (int tubeIndex = 0; tubeIndex < 4; ++tubeIndex)
    {
        const float cx = grille.getX() + grille.getWidth() * (0.2f + 0.2f * static_cast<float>(tubeIndex));
        const float cy = grille.getCentreY();
        graphics.setColour(design::ledWarm().withAlpha(0.18f + 0.05f * static_cast<float>(tubeIndex % 2)));
        graphics.fillEllipse(cx - 10.0f, cy - 14.0f, 20.0f, 28.0f);
        graphics.setColour(design::ledWarm().withAlpha(0.75f));
        graphics.fillEllipse(cx - 4.0f, cy - 6.0f, 8.0f, 12.0f);
    }

    auto controlRail = bounds.reduced(24.0f).removeFromBottom(120.0f);
    graphics.setColour(design::metalDeep().withAlpha(0.55f));
    graphics.fillRoundedRectangle(controlRail, 10.0f);
    graphics.setColour(design::metalBorder().withAlpha(0.8f));
    graphics.drawRoundedRectangle(controlRail, 10.0f, 1.0f);

    graphics.setColour(design::ledWarm());
    graphics.fillEllipse(bounds.getRight() - 48.0f, bounds.getBottom() - 48.0f, 14.0f, 14.0f);
    graphics.setColour(design::ledWarm().withAlpha(0.35f));
    graphics.fillEllipse(bounds.getRight() - 52.0f, bounds.getBottom() - 52.0f, 22.0f, 22.0f);
}

void AmpFaceplateComponent::resized()
{
    auto bounds = getLocalBounds().reduced(32, 28);
    chassisLabel.setBounds(bounds.removeFromTop(28));
    modelStatusLabel.setBounds(bounds.removeFromTop(18));
    bounds.removeFromTop(8);

    auto grilleSpace = bounds.removeFromTop(static_cast<int>(bounds.getHeight() * 0.38f));
    juce::ignoreUnused(grilleSpace);

    modelSlot.setBounds(bounds.removeFromTop(78).reduced(40, 0));
    bounds.removeFromTop(12);

    auto knobs = bounds.removeFromTop(130);
    const int knobWidth = knobs.getWidth() / 3;
    bassKnob.setBounds(knobs.removeFromLeft(knobWidth).reduced(18, 4));
    midKnob.setBounds(knobs.removeFromLeft(knobWidth).reduced(18, 4));
    trebleKnob.setBounds(knobs.reduced(18, 4));

    bounds.removeFromTop(8);
    eqEnableButton.setBounds(bounds.removeFromTop(28).withSizeKeepingCentre(180, 26));
    brandFooterLabel.setBounds(bounds.removeFromBottom(20));
}

FileSlotComponent& AmpFaceplateComponent::getModelSlot() noexcept { return modelSlot; }
KnobComponent& AmpFaceplateComponent::getBassKnob() noexcept { return bassKnob; }
KnobComponent& AmpFaceplateComponent::getMidKnob() noexcept { return midKnob; }
KnobComponent& AmpFaceplateComponent::getTrebleKnob() noexcept { return trebleKnob; }
juce::ToggleButton& AmpFaceplateComponent::getEqEnableButton() noexcept { return eqEnableButton; }

void AmpFaceplateComponent::setModelStatus(const juce::String& modelName, bool isLoaded, bool isLoading)
{
    if (isLoading)
        modelStatusLabel.setText("LOADING MODEL…", juce::dontSendNotification);
    else if (isLoaded)
        modelStatusLabel.setText(modelName.toUpperCase(), juce::dontSendNotification);
    else
        modelStatusLabel.setText("NO MODEL LOADED", juce::dontSendNotification);
}

void AmpFaceplateComponent::paintTubeGlow(juce::Graphics& graphics, juce::Rectangle<float> grilleBounds) const
{
    juce::ColourGradient glow(
        design::ledWarm().withAlpha(0.12f),
        grilleBounds.getCentreX(),
        grilleBounds.getCentreY(),
        juce::Colours::transparentBlack,
        grilleBounds.getCentreX(),
        grilleBounds.getBottom() + 40.0f,
        true);
    graphics.setGradientFill(glow);
    graphics.fillEllipse(grilleBounds.expanded(30.0f, 18.0f));
}

} // namespace lumen::ui
