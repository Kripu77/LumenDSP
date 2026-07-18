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

    modelStatusLabel.setFont(design::sectionFont());
    modelStatusLabel.setColour(juce::Label::textColourId, design::metalTextSecondary());
    modelStatusLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(modelStatusLabel);

    brandFooterLabel.setText("OPEN SOURCE  |  NEURAL AMP MODELER CORE", juce::dontSendNotification);
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
    design::drawSoftShadow(graphics, bounds, static_cast<float>(design::panelCornerRadiusPixels), 0.26f);

    juce::ColourGradient chassis(
        juce::Colour::fromRGB(58, 62, 68),
        bounds.getX(),
        bounds.getY(),
        juce::Colour::fromRGB(24, 26, 30),
        bounds.getX(),
        bounds.getBottom(),
        false);
    graphics.setGradientFill(chassis);
    graphics.fillRoundedRectangle(bounds, static_cast<float>(design::panelCornerRadiusPixels));

    graphics.setColour(juce::Colours::white.withAlpha(0.08f));
    graphics.drawRoundedRectangle(bounds.reduced(1.2f), static_cast<float>(design::panelCornerRadiusPixels) - 1.0f, 1.2f);
    graphics.setColour(juce::Colours::black.withAlpha(0.55f));
    graphics.drawRoundedRectangle(bounds, static_cast<float>(design::panelCornerRadiusPixels), 1.4f);

    auto handle = juce::Rectangle<float>(bounds.getCentreX() - 70.0f, bounds.getY() - 2.0f, 140.0f, 14.0f);
    graphics.setColour(design::metalDeep());
    graphics.fillRoundedRectangle(handle, 6.0f);

    auto grille = bounds.reduced(26.0f, 22.0f);
    grille.setHeight(grille.getHeight() * 0.36f);
    paintTubeGlow(graphics, grille);

    juce::ColourGradient grilleBody(
        juce::Colour::fromRGB(18, 19, 22),
        grille.getX(),
        grille.getY(),
        juce::Colour::fromRGB(10, 11, 13),
        grille.getX(),
        grille.getBottom(),
        false);
    graphics.setGradientFill(grilleBody);
    graphics.fillRoundedRectangle(grille, 8.0f);
    graphics.setColour(design::metalBorder().withAlpha(0.7f));
    graphics.drawRoundedRectangle(grille, 8.0f, 1.2f);

    const int barCount = 28;
    for (int barIndex = 0; barIndex < barCount; ++barIndex)
    {
        const float x = grille.getX() + 8.0f
                        + static_cast<float>(barIndex) * ((grille.getWidth() - 16.0f) / static_cast<float>(barCount - 1));
        graphics.setColour(juce::Colour::fromRGB(8, 8, 10));
        graphics.fillRect(x - 1.4f, grille.getY() + 7.0f, 2.8f, grille.getHeight() - 14.0f);
        graphics.setColour(juce::Colours::white.withAlpha(0.03f));
        graphics.drawLine(x - 0.5f, grille.getY() + 7.0f, x - 0.5f, grille.getBottom() - 7.0f, 1.0f);
    }

    for (int tubeIndex = 0; tubeIndex < 5; ++tubeIndex)
    {
        const float cx = grille.getX() + grille.getWidth() * (0.14f + 0.18f * static_cast<float>(tubeIndex));
        const float cy = grille.getCentreY() + 2.0f;
        graphics.setColour(design::ledWarm().withAlpha(0.10f + 0.03f * static_cast<float>(tubeIndex % 2)));
        graphics.fillEllipse(cx - 14.0f, cy - 18.0f, 28.0f, 36.0f);
        juce::ColourGradient tube(
            design::ledWarm().brighter(0.2f),
            cx,
            cy - 8.0f,
            design::ledWarmDim(),
            cx,
            cy + 10.0f,
            false);
        graphics.setGradientFill(tube);
        graphics.fillEllipse(cx - 5.0f, cy - 9.0f, 10.0f, 18.0f);
        graphics.setColour(juce::Colours::white.withAlpha(0.25f));
        graphics.fillEllipse(cx - 2.0f, cy - 6.0f, 3.0f, 5.0f);
    }

    auto controlBay = bounds.reduced(22.0f);
    controlBay = controlBay.withTrimmedTop(controlBay.getHeight() * 0.48f).withTrimmedBottom(18.0f);
    juce::ColourGradient bay(
        juce::Colour::fromRGB(42, 46, 52),
        controlBay.getX(),
        controlBay.getY(),
        juce::Colour::fromRGB(28, 30, 34),
        controlBay.getX(),
        controlBay.getBottom(),
        false);
    graphics.setGradientFill(bay);
    graphics.fillRoundedRectangle(controlBay, 12.0f);
    graphics.setColour(juce::Colours::black.withAlpha(0.35f));
    graphics.drawRoundedRectangle(controlBay, 12.0f, 1.2f);
    graphics.setColour(juce::Colours::white.withAlpha(0.06f));
    graphics.drawLine(controlBay.getX() + 12.0f, controlBay.getY() + 1.0f, controlBay.getRight() - 12.0f, controlBay.getY() + 1.0f, 1.0f);

    auto jewel = juce::Rectangle<float>(bounds.getRight() - 46.0f, bounds.getBottom() - 46.0f, 16.0f, 16.0f);
    graphics.setColour(design::ledWarm().withAlpha(0.3f));
    graphics.fillEllipse(jewel.expanded(5.0f));
    graphics.setColour(design::ledWarm());
    graphics.fillEllipse(jewel);
    graphics.setColour(juce::Colours::white.withAlpha(0.45f));
    graphics.fillEllipse(jewel.reduced(5.0f).translated(-1.0f, -1.5f));
}

void AmpFaceplateComponent::resized()
{
    auto bounds = getLocalBounds().reduced(30, 26);
    chassisLabel.setBounds(bounds.removeFromTop(24));
    modelStatusLabel.setBounds(bounds.removeFromTop(18));
    bounds.removeFromTop(6);

    bounds.removeFromTop(static_cast<int>(bounds.getHeight() * 0.34f));
    bounds.removeFromTop(10);

    modelSlot.setBounds(bounds.removeFromTop(72).reduced(8, 0));
    bounds.removeFromTop(10);

    auto knobs = bounds.removeFromTop(128);
    const int knobWidth = knobs.getWidth() / 3;
    bassKnob.setBounds(knobs.removeFromLeft(knobWidth).reduced(22, 2));
    midKnob.setBounds(knobs.removeFromLeft(knobWidth).reduced(22, 2));
    trebleKnob.setBounds(knobs.reduced(22, 2));

    bounds.removeFromTop(4);
    eqEnableButton.setBounds(bounds.removeFromTop(26).withSizeKeepingCentre(200, 24));
    brandFooterLabel.setBounds(bounds.removeFromBottom(18));
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
        modelStatusLabel.setText("LOADING MODEL...", juce::dontSendNotification);
        modelStatusLabel.setColour(juce::Label::textColourId, design::accent());
    }
    else if (isLoaded)
    {
        modelStatusLabel.setText(modelName.toUpperCase(), juce::dontSendNotification);
        modelStatusLabel.setColour(juce::Label::textColourId, design::ledWarm());
    }
    else
    {
        modelStatusLabel.setText("NO MODEL LOADED", juce::dontSendNotification);
        modelStatusLabel.setColour(juce::Label::textColourId, design::metalTextMuted());
    }
}

void AmpFaceplateComponent::paintTubeGlow(juce::Graphics& graphics, juce::Rectangle<float> grilleBounds) const
{
    juce::ColourGradient glow(
        design::ledWarm().withAlpha(0.14f),
        grilleBounds.getCentreX(),
        grilleBounds.getCentreY(),
        juce::Colours::transparentBlack,
        grilleBounds.getCentreX(),
        grilleBounds.getBottom() + 50.0f,
        true);
    graphics.setGradientFill(glow);
    graphics.fillEllipse(grilleBounds.expanded(36.0f, 22.0f));
}

} // namespace lumen::ui
