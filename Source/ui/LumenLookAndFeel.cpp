#include "ui/LumenLookAndFeel.h"

namespace lumen::ui
{

LumenLookAndFeel::LumenLookAndFeel()
{
    applyColourScheme();
}

void LumenLookAndFeel::applyColourScheme()
{
    setColour(juce::ResizableWindow::backgroundColourId, design::studioBackdropBottom());
    setColour(juce::DocumentWindow::backgroundColourId, design::studioBackdropBottom());

    setColour(juce::TextButton::buttonColourId, design::chromeSurfaceRaised());
    setColour(juce::TextButton::buttonOnColourId, design::accentSoft());
    setColour(juce::TextButton::textColourOffId, design::chromeTextPrimary());
    setColour(juce::TextButton::textColourOnId, design::chromeTextPrimary());

    setColour(juce::ComboBox::backgroundColourId, design::chromeSurfaceRaised());
    setColour(juce::ComboBox::outlineColourId, design::chromeBorder());
    setColour(juce::ComboBox::textColourId, design::chromeTextPrimary());
    setColour(juce::ComboBox::arrowColourId, design::accent());

    setColour(juce::PopupMenu::backgroundColourId, design::chromeSurfaceRaised());
    setColour(juce::PopupMenu::textColourId, design::chromeTextPrimary());
    setColour(juce::PopupMenu::highlightedBackgroundColourId, design::accentSoft());
    setColour(juce::PopupMenu::highlightedTextColourId, design::chromeTextPrimary());

    setColour(juce::Label::textColourId, design::chromeTextSecondary());
    setColour(juce::Label::backgroundColourId, juce::Colours::transparentBlack);

    setColour(juce::ToggleButton::textColourId, design::chromeTextSecondary());
    setColour(juce::ToggleButton::tickColourId, design::ledWarm());
    setColour(juce::ToggleButton::tickDisabledColourId, design::chromeTextMuted());

    setColour(juce::Slider::rotarySliderFillColourId, design::accent());
    setColour(juce::Slider::rotarySliderOutlineColourId, design::chromeBorder());
    setColour(juce::Slider::thumbColourId, design::chromeSurfaceRaised());
    setColour(juce::Slider::trackColourId, design::metalRaised());
    setColour(juce::Slider::backgroundColourId, design::metalDeep());
    setColour(juce::Slider::textBoxTextColourId, design::chromeTextPrimary());
    setColour(juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
    setColour(juce::Slider::textBoxBackgroundColourId, juce::Colours::transparentBlack);

    setColour(juce::ScrollBar::thumbColourId, design::accentDim());
    setColour(juce::TextEditor::backgroundColourId, design::chromeSurfaceRaised());
    setColour(juce::TextEditor::outlineColourId, design::chromeBorder());
    setColour(juce::TextEditor::focusedOutlineColourId, design::accent());
    setColour(juce::TextEditor::textColourId, design::chromeTextPrimary());
    setColour(juce::CaretComponent::caretColourId, design::accent());
}

void LumenLookAndFeel::drawRotarySlider(
    juce::Graphics& graphics,
    int x,
    int y,
    int width,
    int height,
    float sliderPosProportional,
    float rotaryStartAngle,
    float rotaryEndAngle,
    juce::Slider& slider)
{
    auto bounds = juce::Rectangle<float>(
                      static_cast<float>(x),
                      static_cast<float>(y),
                      static_cast<float>(width),
                      static_cast<float>(height))
                      .reduced(3.0f);

    const bool useMetalStyle = slider.getProperties()["metalStyle"];
    if (useMetalStyle)
        drawMetalRotary(graphics, bounds, sliderPosProportional, rotaryStartAngle, rotaryEndAngle);
    else
        drawChromeRotary(graphics, bounds, sliderPosProportional, rotaryStartAngle, rotaryEndAngle);
}

void LumenLookAndFeel::drawChromeRotary(
    juce::Graphics& graphics,
    juce::Rectangle<float> bounds,
    float sliderPosProportional,
    float rotaryStartAngle,
    float rotaryEndAngle)
{
    const auto radius = juce::jmin(bounds.getWidth(), bounds.getHeight()) * 0.5f;
    const auto center = bounds.getCentre();
    const auto angle = rotaryStartAngle + sliderPosProportional * (rotaryEndAngle - rotaryStartAngle);
    const auto arcRadius = radius - static_cast<float>(design::knobValueArcThicknessPixels);

    juce::Path backgroundArc;
    backgroundArc.addCentredArc(center.x, center.y, arcRadius, arcRadius, 0.0f, rotaryStartAngle, rotaryEndAngle, true);
    graphics.setColour(design::chromeBorder());
    graphics.strokePath(
        backgroundArc,
        juce::PathStrokeType(
            static_cast<float>(design::knobValueArcThicknessPixels),
            juce::PathStrokeType::curved,
            juce::PathStrokeType::rounded));

    juce::Path valueArc;
    valueArc.addCentredArc(center.x, center.y, arcRadius, arcRadius, 0.0f, rotaryStartAngle, angle, true);
    graphics.setColour(design::accent());
    graphics.strokePath(
        valueArc,
        juce::PathStrokeType(
            static_cast<float>(design::knobValueArcThicknessPixels),
            juce::PathStrokeType::curved,
            juce::PathStrokeType::rounded));

    const auto knobRadius = arcRadius - 7.0f;
    juce::ColourGradient cap(
        design::chromeSurfaceRaised(),
        center.x - knobRadius * 0.4f,
        center.y - knobRadius * 0.5f,
        design::chromeBorder().brighter(0.15f),
        center.x + knobRadius * 0.3f,
        center.y + knobRadius * 0.5f,
        false);
    graphics.setGradientFill(cap);
    graphics.fillEllipse(center.x - knobRadius, center.y - knobRadius, knobRadius * 2.0f, knobRadius * 2.0f);
    graphics.setColour(design::chromeBorder());
    graphics.drawEllipse(center.x - knobRadius, center.y - knobRadius, knobRadius * 2.0f, knobRadius * 2.0f, 1.0f);

    juce::Path pointer;
    pointer.addRoundedRectangle(
        -static_cast<float>(design::knobPointerThicknessPixels) * 0.5f,
        -knobRadius + 5.0f,
        static_cast<float>(design::knobPointerThicknessPixels),
        static_cast<float>(design::knobPointerLengthPixels),
        1.0f);
    pointer.applyTransform(juce::AffineTransform::rotation(angle).translated(center.x, center.y));
    graphics.setColour(design::chromeTextPrimary());
    graphics.fillPath(pointer);
}

void LumenLookAndFeel::drawMetalRotary(
    juce::Graphics& graphics,
    juce::Rectangle<float> bounds,
    float sliderPosProportional,
    float rotaryStartAngle,
    float rotaryEndAngle)
{
    const auto radius = juce::jmin(bounds.getWidth(), bounds.getHeight()) * 0.5f;
    const auto center = bounds.getCentre();
    const auto angle = rotaryStartAngle + sliderPosProportional * (rotaryEndAngle - rotaryStartAngle);
    const auto knobRadius = radius - 4.0f;

    graphics.setColour(juce::Colours::black.withAlpha(0.35f));
    graphics.fillEllipse(center.x - knobRadius + 1.5f, center.y - knobRadius + 2.5f, knobRadius * 2.0f, knobRadius * 2.0f);

    juce::ColourGradient cap(
        design::metalRaised().brighter(0.12f),
        center.x - knobRadius * 0.35f,
        center.y - knobRadius * 0.45f,
        design::metalDeep(),
        center.x + knobRadius * 0.25f,
        center.y + knobRadius * 0.5f,
        false);
    graphics.setGradientFill(cap);
    graphics.fillEllipse(center.x - knobRadius, center.y - knobRadius, knobRadius * 2.0f, knobRadius * 2.0f);

    graphics.setColour(design::metalBorder());
    graphics.drawEllipse(center.x - knobRadius, center.y - knobRadius, knobRadius * 2.0f, knobRadius * 2.0f, 1.2f);

    const auto indicatorRadius = 3.0f;
    const auto indicatorDistance = knobRadius - 10.0f;
    const auto indicatorX = center.x + indicatorDistance * std::sin(angle);
    const auto indicatorY = center.y - indicatorDistance * std::cos(angle);
    graphics.setColour(design::ledWarm());
    graphics.fillEllipse(indicatorX - indicatorRadius, indicatorY - indicatorRadius, indicatorRadius * 2.0f, indicatorRadius * 2.0f);
    graphics.setColour(design::ledWarm().withAlpha(design::ledGlowAlpha));
    graphics.fillEllipse(
        indicatorX - indicatorRadius * 2.2f,
        indicatorY - indicatorRadius * 2.2f,
        indicatorRadius * 4.4f,
        indicatorRadius * 4.4f);

    juce::ignoreUnused(sliderPosProportional, rotaryStartAngle, rotaryEndAngle);
}

void LumenLookAndFeel::drawLinearSlider(
    juce::Graphics& graphics,
    int x,
    int y,
    int width,
    int height,
    float sliderPos,
    float minSliderPos,
    float maxSliderPos,
    juce::Slider::SliderStyle style,
    juce::Slider& slider)
{
    juce::ignoreUnused(minSliderPos, maxSliderPos, slider);

    if (style != juce::Slider::LinearVertical)
    {
        LookAndFeel_V4::drawLinearSlider(graphics, x, y, width, height, sliderPos, minSliderPos, maxSliderPos, style, slider);
        return;
    }

    auto track = juce::Rectangle<float>(
                     static_cast<float>(x),
                     static_cast<float>(y),
                     static_cast<float>(width),
                     static_cast<float>(height))
                     .withSizeKeepingCentre(8.0f, static_cast<float>(height) - 8.0f);

    graphics.setColour(design::metalDeep());
    graphics.fillRoundedRectangle(track, 4.0f);
    graphics.setColour(design::metalBorder());
    graphics.drawRoundedRectangle(track, 4.0f, 1.0f);

    const float thumbHeight = 18.0f;
    const float thumbWidth = static_cast<float>(width) - 4.0f;
    const float thumbY = sliderPos - thumbHeight * 0.5f;
    auto thumb = juce::Rectangle<float>(
        static_cast<float>(x) + 2.0f,
        thumbY,
        thumbWidth,
        thumbHeight);

    graphics.setColour(design::metalRaised());
    graphics.fillRoundedRectangle(thumb, 4.0f);
    graphics.setColour(design::ledWarm());
    graphics.fillEllipse(thumb.getCentreX() - 3.0f, thumb.getCentreY() - 3.0f, 6.0f, 6.0f);
    graphics.setColour(design::metalBorder());
    graphics.drawRoundedRectangle(thumb, 4.0f, 1.0f);
}

void LumenLookAndFeel::drawButtonBackground(
    juce::Graphics& graphics,
    juce::Button& button,
    const juce::Colour& backgroundColour,
    bool shouldDrawButtonAsHighlighted,
    bool shouldDrawButtonAsDown)
{
    auto bounds = button.getLocalBounds().toFloat().reduced(0.5f);
    auto fillColour = backgroundColour;

    if (shouldDrawButtonAsDown)
        fillColour = design::accentSoft();
    else if (shouldDrawButtonAsHighlighted)
        fillColour = fillColour.brighter(design::controlHoverBrightnessLift);
    else if (button.getToggleState())
        fillColour = design::accentSoft();

    graphics.setColour(fillColour);
    graphics.fillRoundedRectangle(bounds, static_cast<float>(design::controlCornerRadiusPixels));
    graphics.setColour(button.getToggleState() || button.hasKeyboardFocus(true) ? design::accent() : design::chromeBorder());
    graphics.drawRoundedRectangle(bounds, static_cast<float>(design::controlCornerRadiusPixels), 1.0f);
}

void LumenLookAndFeel::drawButtonText(
    juce::Graphics& graphics,
    juce::TextButton& button,
    bool shouldDrawButtonAsHighlighted,
    bool shouldDrawButtonAsDown)
{
    juce::ignoreUnused(shouldDrawButtonAsHighlighted, shouldDrawButtonAsDown);
    graphics.setFont(getTextButtonFont(button, button.getHeight()));
    graphics.setColour(button.isEnabled() ? design::chromeTextPrimary() : design::chromeTextMuted());
    graphics.drawFittedText(
        button.getButtonText(),
        button.getLocalBounds().reduced(design::spacingUnitPixels / 2),
        juce::Justification::centred,
        1);
}

void LumenLookAndFeel::drawComboBox(
    juce::Graphics& graphics,
    int width,
    int height,
    bool isButtonDown,
    int buttonX,
    int buttonY,
    int buttonW,
    int buttonH,
    juce::ComboBox& comboBox)
{
    juce::ignoreUnused(isButtonDown, buttonX, buttonY, buttonW, buttonH, comboBox);

    auto bounds = juce::Rectangle<float>(0.0f, 0.0f, static_cast<float>(width), static_cast<float>(height)).reduced(0.5f);
    graphics.setColour(design::chromeSurfaceRaised());
    graphics.fillRoundedRectangle(bounds, static_cast<float>(design::controlCornerRadiusPixels));
    graphics.setColour(design::chromeBorder());
    graphics.drawRoundedRectangle(bounds, static_cast<float>(design::controlCornerRadiusPixels), 1.0f);

    const auto arrowZone = juce::Rectangle<float>(static_cast<float>(width - 24), 0.0f, 18.0f, static_cast<float>(height));
    juce::Path arrow;
    arrow.addTriangle(
        arrowZone.getX() + 3.0f,
        arrowZone.getCentreY() - 3.0f,
        arrowZone.getRight() - 3.0f,
        arrowZone.getCentreY() - 3.0f,
        arrowZone.getCentreX(),
        arrowZone.getCentreY() + 4.0f);
    graphics.setColour(design::accent());
    graphics.fillPath(arrow);
}

void LumenLookAndFeel::drawPopupMenuBackground(juce::Graphics& graphics, int width, int height)
{
    graphics.fillAll(design::chromeSurfaceRaised());
    graphics.setColour(design::chromeBorder());
    graphics.drawRect(0, 0, width, height, 1);
}

void LumenLookAndFeel::drawPopupMenuItem(
    juce::Graphics& graphics,
    const juce::Rectangle<int>& area,
    bool isSeparator,
    bool isActive,
    bool isHighlighted,
    bool isTicked,
    bool hasSubMenu,
    const juce::String& text,
    const juce::String& shortcutKeyText,
    const juce::Drawable* icon,
    const juce::Colour* textColour)
{
    juce::ignoreUnused(hasSubMenu, shortcutKeyText, icon, textColour);

    if (isSeparator)
    {
        auto separatorArea = area.reduced(design::spacingUnitPixels, 0);
        graphics.setColour(design::chromeBorder());
        graphics.fillRect(separatorArea.withHeight(1).withY(area.getCentreY()));
        return;
    }

    auto itemArea = area.reduced(1);
    if (isHighlighted && isActive)
    {
        graphics.setColour(design::accentSoft());
        graphics.fillRoundedRectangle(itemArea.toFloat(), 6.0f);
    }

    graphics.setColour(isActive ? design::chromeTextPrimary() : design::chromeTextMuted());
    graphics.setFont(getPopupMenuFont());
    auto textArea = itemArea.reduced(design::spacingUnitPixels, 0);

    if (isTicked)
    {
        graphics.setColour(design::accent());
        graphics.fillEllipse(static_cast<float>(textArea.getX()), static_cast<float>(textArea.getCentreY() - 3), 6.0f, 6.0f);
        textArea.removeFromLeft(design::spacingUnitPixels + 4);
    }

    graphics.drawFittedText(text, textArea, juce::Justification::centredLeft, 1);
}

void LumenLookAndFeel::drawToggleButton(
    juce::Graphics& graphics,
    juce::ToggleButton& button,
    bool shouldDrawButtonAsHighlighted,
    bool shouldDrawButtonAsDown)
{
    juce::ignoreUnused(shouldDrawButtonAsDown);

    const auto bounds = button.getLocalBounds().toFloat();
    const float trackWidth = 36.0f;
    const float trackHeight = 20.0f;
    const auto trackBounds = juce::Rectangle<float>(
        bounds.getX(),
        bounds.getCentreY() - trackHeight * 0.5f,
        trackWidth,
        trackHeight);

    const bool isOn = button.getToggleState();
    auto trackColour = isOn ? design::ledWarmDim() : design::chromeBorder();
    if (shouldDrawButtonAsHighlighted)
        trackColour = trackColour.brighter(design::controlHoverBrightnessLift);

    graphics.setColour(trackColour);
    graphics.fillRoundedRectangle(trackBounds, trackHeight * 0.5f);
    graphics.setColour(design::chromeBorder().darker(0.05f));
    graphics.drawRoundedRectangle(trackBounds, trackHeight * 0.5f, 1.0f);

    const float thumbDiameter = 16.0f;
    const float thumbTravel = trackWidth - thumbDiameter - 4.0f;
    const float thumbX = trackBounds.getX() + 2.0f + (isOn ? thumbTravel : 0.0f);
    const float thumbY = trackBounds.getCentreY() - thumbDiameter * 0.5f;
    graphics.setColour(isOn ? design::ledWarm() : design::chromeSurfaceRaised());
    graphics.fillEllipse(thumbX, thumbY, thumbDiameter, thumbDiameter);

    graphics.setColour(button.isEnabled() ? design::chromeTextSecondary() : design::chromeTextMuted());
    graphics.setFont(design::bodyFont());
    graphics.drawFittedText(
        button.getButtonText(),
        button.getLocalBounds().withTrimmedLeft(static_cast<int>(trackWidth) + design::spacingUnitPixels),
        juce::Justification::centredLeft,
        1);
}

void LumenLookAndFeel::drawLabel(juce::Graphics& graphics, juce::Label& label)
{
    graphics.setColour(label.findColour(juce::Label::textColourId));
    graphics.setFont(getLabelFont(label));
    graphics.drawFittedText(
        label.getText(),
        label.getLocalBounds(),
        label.getJustificationType(),
        juce::jmax(1, static_cast<int>(static_cast<float>(label.getHeight()) / graphics.getCurrentFont().getHeight())));
}

juce::Font LumenLookAndFeel::getTextButtonFont(juce::TextButton&, int)
{
    return design::bodyFont();
}

juce::Font LumenLookAndFeel::getComboBoxFont(juce::ComboBox&)
{
    return design::bodyFont();
}

juce::Font LumenLookAndFeel::getLabelFont(juce::Label&)
{
    return design::bodyFont();
}

juce::Font LumenLookAndFeel::getPopupMenuFont()
{
    return design::bodyFont();
}

} // namespace lumen::ui
