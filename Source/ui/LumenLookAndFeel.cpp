#include "ui/LumenLookAndFeel.h"

namespace lumen::ui
{

LumenLookAndFeel::LumenLookAndFeel()
{
    applyColourScheme();
}

void LumenLookAndFeel::applyColourScheme()
{
    setColour(juce::ResizableWindow::backgroundColourId, design::backgroundDeep());
    setColour(juce::DocumentWindow::backgroundColourId, design::backgroundDeep());

    setColour(juce::TextButton::buttonColourId, design::backgroundElevated());
    setColour(juce::TextButton::buttonOnColourId, design::accentDim());
    setColour(juce::TextButton::textColourOffId, design::textPrimary());
    setColour(juce::TextButton::textColourOnId, design::textPrimary());

    setColour(juce::ComboBox::backgroundColourId, design::backgroundElevated());
    setColour(juce::ComboBox::outlineColourId, design::borderSubtle());
    setColour(juce::ComboBox::textColourId, design::textPrimary());
    setColour(juce::ComboBox::arrowColourId, design::accent());

    setColour(juce::PopupMenu::backgroundColourId, design::backgroundPanel());
    setColour(juce::PopupMenu::textColourId, design::textPrimary());
    setColour(juce::PopupMenu::highlightedBackgroundColourId, design::accentSoft());
    setColour(juce::PopupMenu::highlightedTextColourId, design::textPrimary());

    setColour(juce::Label::textColourId, design::textSecondary());
    setColour(juce::Label::backgroundColourId, juce::Colours::transparentBlack);

    setColour(juce::ToggleButton::textColourId, design::textSecondary());
    setColour(juce::ToggleButton::tickColourId, design::accent());
    setColour(juce::ToggleButton::tickDisabledColourId, design::textMuted());

    setColour(juce::Slider::rotarySliderFillColourId, design::accent());
    setColour(juce::Slider::rotarySliderOutlineColourId, design::borderSubtle());
    setColour(juce::Slider::thumbColourId, design::textPrimary());
    setColour(juce::Slider::textBoxTextColourId, design::textPrimary());
    setColour(juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
    setColour(juce::Slider::textBoxBackgroundColourId, juce::Colours::transparentBlack);

    setColour(juce::ScrollBar::thumbColourId, design::accentDim());
    setColour(juce::TextEditor::backgroundColourId, design::backgroundElevated());
    setColour(juce::TextEditor::outlineColourId, design::borderSubtle());
    setColour(juce::TextEditor::focusedOutlineColourId, design::accent());
    setColour(juce::TextEditor::textColourId, design::textPrimary());
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
    juce::ignoreUnused(slider);

    const auto bounds = juce::Rectangle<float>(
                            static_cast<float>(x),
                            static_cast<float>(y),
                            static_cast<float>(width),
                            static_cast<float>(height))
                            .reduced(4.0f);
    const auto radius = juce::jmin(bounds.getWidth(), bounds.getHeight()) * 0.5f;
    const auto center = bounds.getCentre();
    const auto angle = rotaryStartAngle + sliderPosProportional * (rotaryEndAngle - rotaryStartAngle);
    const auto arcRadius = radius - static_cast<float>(design::knobValueArcThicknessPixels);

    juce::Path backgroundArc;
    backgroundArc.addCentredArc(
        center.x,
        center.y,
        arcRadius,
        arcRadius,
        0.0f,
        rotaryStartAngle,
        rotaryEndAngle,
        true);

    graphics.setColour(design::borderSubtle());
    graphics.strokePath(
        backgroundArc,
        juce::PathStrokeType(
            static_cast<float>(design::knobValueArcThicknessPixels),
            juce::PathStrokeType::curved,
            juce::PathStrokeType::rounded));

    juce::Path valueArc;
    valueArc.addCentredArc(
        center.x,
        center.y,
        arcRadius,
        arcRadius,
        0.0f,
        rotaryStartAngle,
        angle,
        true);

    graphics.setColour(design::accent());
    graphics.strokePath(
        valueArc,
        juce::PathStrokeType(
            static_cast<float>(design::knobValueArcThicknessPixels),
            juce::PathStrokeType::curved,
            juce::PathStrokeType::rounded));

    const auto knobRadius = arcRadius - 8.0f;
    graphics.setColour(design::backgroundControl());
    graphics.fillEllipse(center.x - knobRadius, center.y - knobRadius, knobRadius * 2.0f, knobRadius * 2.0f);
    graphics.setColour(design::borderSubtle());
    graphics.drawEllipse(center.x - knobRadius, center.y - knobRadius, knobRadius * 2.0f, knobRadius * 2.0f, 1.0f);

    juce::Path pointer;
    const float pointerLength = static_cast<float>(design::knobPointerLengthPixels);
    const float pointerThickness = static_cast<float>(design::knobPointerThicknessPixels);
    pointer.addRoundedRectangle(-pointerThickness * 0.5f, -knobRadius + 4.0f, pointerThickness, pointerLength, 1.0f);
    pointer.applyTransform(juce::AffineTransform::rotation(angle).translated(center.x, center.y));
    graphics.setColour(design::textPrimary());
    graphics.fillPath(pointer);
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
        fillColour = design::accentDim();
    else if (shouldDrawButtonAsHighlighted)
        fillColour = fillColour.brighter(design::controlHoverBrightnessLift);
    else if (button.getToggleState())
        fillColour = design::accentDim();

    graphics.setColour(fillColour);
    graphics.fillRoundedRectangle(bounds, static_cast<float>(design::controlCornerRadiusPixels));
    graphics.setColour(button.hasKeyboardFocus(true) ? design::accent() : design::borderSubtle());
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
    graphics.setColour(
        button.isEnabled() ? design::textPrimary() : design::textMuted());
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

    auto bounds = juce::Rectangle<float>(0.0f, 0.0f, static_cast<float>(width), static_cast<float>(height))
                      .reduced(0.5f);
    graphics.setColour(design::backgroundElevated());
    graphics.fillRoundedRectangle(bounds, static_cast<float>(design::controlCornerRadiusPixels));
    graphics.setColour(design::borderSubtle());
    graphics.drawRoundedRectangle(bounds, static_cast<float>(design::controlCornerRadiusPixels), 1.0f);

    const auto arrowZone = juce::Rectangle<float>(
        static_cast<float>(width - 24),
        0.0f,
        18.0f,
        static_cast<float>(height));
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
    graphics.fillAll(design::backgroundPanel());
    graphics.setColour(design::borderSubtle());
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
        graphics.setColour(design::borderSubtle());
        graphics.fillRect(separatorArea.withHeight(1).withY(area.getCentreY()));
        return;
    }

    auto itemArea = area.reduced(1);

    if (isHighlighted && isActive)
    {
        graphics.setColour(design::accentSoft());
        graphics.fillRoundedRectangle(
            itemArea.toFloat(),
            static_cast<float>(design::controlCornerRadiusPixels) * 0.5f);
    }

    graphics.setColour(isActive ? design::textPrimary() : design::textMuted());
    graphics.setFont(getPopupMenuFont());

    auto textArea = itemArea.reduced(design::spacingUnitPixels, 0);

    if (isTicked)
    {
        graphics.setColour(design::accent());
        graphics.fillEllipse(
            static_cast<float>(textArea.getX()),
            static_cast<float>(textArea.getCentreY() - 3),
            6.0f,
            6.0f);
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
    const float trackWidth = 34.0f;
    const float trackHeight = 18.0f;
    const auto trackBounds = juce::Rectangle<float>(
        bounds.getX(),
        bounds.getCentreY() - trackHeight * 0.5f,
        trackWidth,
        trackHeight);

    const bool isOn = button.getToggleState();
    auto trackColour = isOn ? design::accentDim() : design::backgroundControl();

    if (shouldDrawButtonAsHighlighted)
        trackColour = trackColour.brighter(design::controlHoverBrightnessLift);

    graphics.setColour(trackColour);
    graphics.fillRoundedRectangle(trackBounds, trackHeight * 0.5f);
    graphics.setColour(design::borderSubtle());
    graphics.drawRoundedRectangle(trackBounds, trackHeight * 0.5f, 1.0f);

    const float thumbDiameter = 14.0f;
    const float thumbTravel = trackWidth - thumbDiameter - 4.0f;
    const float thumbX = trackBounds.getX() + 2.0f + (isOn ? thumbTravel : 0.0f);
    const float thumbY = trackBounds.getCentreY() - thumbDiameter * 0.5f;
    graphics.setColour(isOn ? design::accent() : design::textSecondary());
    graphics.fillEllipse(thumbX, thumbY, thumbDiameter, thumbDiameter);

    graphics.setColour(button.isEnabled() ? design::textSecondary() : design::textMuted());
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

juce::Font LumenLookAndFeel::getTextButtonFont(juce::TextButton& button, int buttonHeight)
{
    juce::ignoreUnused(button, buttonHeight);
    return design::bodyFont();
}

juce::Font LumenLookAndFeel::getComboBoxFont(juce::ComboBox& comboBox)
{
    juce::ignoreUnused(comboBox);
    return design::bodyFont();
}

juce::Font LumenLookAndFeel::getLabelFont(juce::Label& label)
{
    juce::ignoreUnused(label);
    return design::bodyFont();
}

juce::Font LumenLookAndFeel::getPopupMenuFont()
{
    return design::bodyFont();
}

} // namespace lumen::ui
