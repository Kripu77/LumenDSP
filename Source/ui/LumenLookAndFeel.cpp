#include "ui/LumenLookAndFeel.h"

namespace lumen::ui
{

LumenLookAndFeel::LumenLookAndFeel()
{
    applyColourScheme();
}

void LumenLookAndFeel::applyColourScheme()
{
    setColour(juce::ResizableWindow::backgroundColourId, design::bgPrimary());
    setColour(juce::DocumentWindow::backgroundColourId, design::bgPrimary());

    setColour(juce::TextButton::buttonColourId, design::bgElevated());
    setColour(juce::TextButton::buttonOnColourId, design::accentSoft());
    setColour(juce::TextButton::textColourOffId, design::textPrimary());
    setColour(juce::TextButton::textColourOnId, design::textPrimary());

    setColour(juce::ComboBox::backgroundColourId, design::bgElevated());
    setColour(juce::ComboBox::outlineColourId, design::borderLight());
    setColour(juce::ComboBox::textColourId, design::textPrimary());
    setColour(juce::ComboBox::arrowColourId, design::accent());

    setColour(juce::PopupMenu::backgroundColourId, design::bgElevated());
    setColour(juce::PopupMenu::textColourId, design::textPrimary());
    setColour(juce::PopupMenu::highlightedBackgroundColourId, design::accentSoft());
    setColour(juce::PopupMenu::highlightedTextColourId, design::textPrimary());

    setColour(juce::Label::textColourId, design::textSecondary());
    setColour(juce::Label::backgroundColourId, juce::Colours::transparentBlack);

    setColour(juce::ToggleButton::textColourId, design::textSecondary());
    setColour(juce::ToggleButton::tickColourId, design::accent());
    setColour(juce::ToggleButton::tickDisabledColourId, design::textMuted());

    setColour(juce::Slider::rotarySliderFillColourId, design::knobRing());
    setColour(juce::Slider::rotarySliderOutlineColourId, design::knobRingTrack());
    setColour(juce::Slider::thumbColourId, design::knobIndicator());
    setColour(juce::Slider::trackColourId, design::bgElevated());
    setColour(juce::Slider::backgroundColourId, design::bgPrimary());
    setColour(juce::Slider::textBoxTextColourId, design::textPrimary());
    setColour(juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
    setColour(juce::Slider::textBoxBackgroundColourId, juce::Colours::transparentBlack);

    setColour(juce::TextEditor::backgroundColourId, design::bgElevated());
    setColour(juce::TextEditor::outlineColourId, design::borderLight());
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

    auto bounds = juce::Rectangle<float>(
                      static_cast<float>(x),
                      static_cast<float>(y),
                      static_cast<float>(width),
                      static_cast<float>(height))
                      .reduced(2.0f);

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
    drawMetalRotary(graphics, bounds, sliderPosProportional, rotaryStartAngle, rotaryEndAngle);
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
    const float ringThickness = 3.5f;
    const float arcRadius = radius - ringThickness * 0.5f;

    juce::Path trackArc;
    trackArc.addCentredArc(center.x, center.y, arcRadius, arcRadius, 0.0f, rotaryStartAngle, rotaryEndAngle, true);
    graphics.setColour(design::knobRingTrack());
    graphics.strokePath(trackArc, juce::PathStrokeType(ringThickness, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

    juce::Path valueArc;
    valueArc.addCentredArc(center.x, center.y, arcRadius, arcRadius, 0.0f, rotaryStartAngle, angle, true);
    graphics.setColour(design::knobRing());
    graphics.strokePath(valueArc, juce::PathStrokeType(ringThickness, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

    const float bodyRadius = radius - ringThickness - 1.5f;
    graphics.setColour(juce::Colours::black.withAlpha(0.45f));
    graphics.fillEllipse(center.x - bodyRadius + 1.5f, center.y - bodyRadius + 2.5f, bodyRadius * 2.0f, bodyRadius * 2.0f);

    juce::ColourGradient body(
        design::knobBodyFrom(),
        center.x,
        center.y - bodyRadius,
        design::knobBodyTo(),
        center.x,
        center.y + bodyRadius,
        false);
    graphics.setGradientFill(body);
    graphics.fillEllipse(center.x - bodyRadius, center.y - bodyRadius, bodyRadius * 2.0f, bodyRadius * 2.0f);

    graphics.setColour(juce::Colours::white.withAlpha(0.08f));
    graphics.drawEllipse(center.x - bodyRadius + 1.0f, center.y - bodyRadius + 1.0f, (bodyRadius - 1.0f) * 2.0f, (bodyRadius - 1.0f) * 2.0f, 1.0f);
    graphics.setColour(design::borderStrong());
    graphics.drawEllipse(center.x - bodyRadius, center.y - bodyRadius, bodyRadius * 2.0f, bodyRadius * 2.0f, 1.0f);

    juce::Path pointer;
    const float pointerLength = bodyRadius * 0.42f;
    pointer.addRoundedRectangle(-1.0f, -bodyRadius + 5.0f, 2.0f, pointerLength, 1.0f);
    pointer.applyTransform(juce::AffineTransform::rotation(angle).translated(center.x, center.y));
    graphics.setColour(design::knobIndicator());
    graphics.fillPath(pointer);

    graphics.setColour(design::knobBodyTo().brighter(0.15f));
    graphics.fillEllipse(center.x - 3.0f, center.y - 3.0f, 6.0f, 6.0f);
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

    constexpr float trackWidthPixels = 6.0f;
    constexpr float thumbWidthPixels = 24.0f;
    constexpr float thumbHeightPixels = 14.0f;

    auto bounds = juce::Rectangle<float>(static_cast<float>(x), static_cast<float>(y), static_cast<float>(width), static_cast<float>(height));
    auto track = bounds.withSizeKeepingCentre(trackWidthPixels, bounds.getHeight() - 10.0f);

    graphics.setColour(design::bgPrimary());
    graphics.fillRoundedRectangle(track, 3.0f);
    graphics.setColour(design::borderStrong());
    graphics.drawRoundedRectangle(track, 3.0f, 1.0f);

    auto fill = juce::Rectangle<float>(track.getX(), sliderPos, track.getWidth(), track.getBottom() - sliderPos);
    graphics.setColour(design::knobRing().withAlpha(0.75f));
    graphics.fillRoundedRectangle(fill, 3.0f);

    auto thumb = juce::Rectangle<float>(
        bounds.getCentreX() - thumbWidthPixels * 0.5f,
        sliderPos - thumbHeightPixels * 0.5f,
        thumbWidthPixels,
        thumbHeightPixels);

    juce::ColourGradient thumbGradient(
        design::knobBodyFrom(),
        thumb.getCentreX(),
        thumb.getY(),
        design::knobBodyTo(),
        thumb.getCentreX(),
        thumb.getBottom(),
        false);
    graphics.setGradientFill(thumbGradient);
    graphics.fillRoundedRectangle(thumb, 4.0f);
    graphics.setColour(design::accent());
    graphics.fillEllipse(thumb.getCentreX() - 2.5f, thumb.getCentreY() - 2.5f, 5.0f, 5.0f);
    graphics.setColour(design::borderStrong());
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
    auto fill = backgroundColour;

    if (shouldDrawButtonAsDown || button.getToggleState())
        fill = design::accentSoft();
    else if (shouldDrawButtonAsHighlighted)
        fill = fill.brighter(design::controlHoverBrightnessLift);

    graphics.setColour(fill);
    graphics.fillRoundedRectangle(bounds, static_cast<float>(design::controlCornerRadiusPixels));
    graphics.setColour(button.getToggleState() ? design::accent() : design::borderLight());
    graphics.drawRoundedRectangle(bounds, static_cast<float>(design::controlCornerRadiusPixels), 1.0f);
}

void LumenLookAndFeel::drawButtonText(
    juce::Graphics& graphics,
    juce::TextButton& button,
    bool,
    bool)
{
    graphics.setFont(getTextButtonFont(button, button.getHeight()));
    graphics.setColour(button.isEnabled() ? design::textPrimary() : design::textMuted());
    graphics.drawFittedText(
        button.getButtonText(),
        button.getLocalBounds().reduced(4),
        juce::Justification::centred,
        1);
}

void LumenLookAndFeel::drawComboBox(
    juce::Graphics& graphics,
    int width,
    int height,
    bool,
    int,
    int,
    int,
    int,
    juce::ComboBox&)
{
    auto bounds = juce::Rectangle<float>(0.0f, 0.0f, static_cast<float>(width), static_cast<float>(height)).reduced(0.5f);
    graphics.setColour(design::bgElevated());
    graphics.fillRoundedRectangle(bounds, static_cast<float>(design::controlCornerRadiusPixels));
    graphics.setColour(design::borderLight());
    graphics.drawRoundedRectangle(bounds, static_cast<float>(design::controlCornerRadiusPixels), 1.0f);

    juce::Path arrow;
    const float cx = static_cast<float>(width) - 14.0f;
    const float cy = static_cast<float>(height) * 0.5f;
    arrow.addTriangle(cx - 4.0f, cy - 2.0f, cx + 4.0f, cy - 2.0f, cx, cy + 4.0f);
    graphics.setColour(design::accent());
    graphics.fillPath(arrow);
}

void LumenLookAndFeel::drawPopupMenuBackground(juce::Graphics& graphics, int width, int height)
{
    graphics.fillAll(design::bgElevated());
    graphics.setColour(design::borderLight());
    graphics.drawRect(0, 0, width, height, 1);
}

void LumenLookAndFeel::drawPopupMenuItem(
    juce::Graphics& graphics,
    const juce::Rectangle<int>& area,
    bool isSeparator,
    bool isActive,
    bool isHighlighted,
    bool isTicked,
    bool,
    const juce::String& text,
    const juce::String&,
    const juce::Drawable*,
    const juce::Colour*)
{
    if (isSeparator)
    {
        graphics.setColour(design::borderLight());
        graphics.fillRect(area.reduced(8, 0).withHeight(1).withY(area.getCentreY()));
        return;
    }

    if (isHighlighted && isActive)
    {
        graphics.setColour(design::accentSoft());
        graphics.fillRoundedRectangle(area.toFloat().reduced(1.0f), 6.0f);
    }

    graphics.setColour(isActive ? design::textPrimary() : design::textMuted());
    graphics.setFont(getPopupMenuFont());
    auto textArea = area.reduced(10, 0);
    if (isTicked)
    {
        graphics.setColour(design::success());
        graphics.fillEllipse(static_cast<float>(textArea.getX()), static_cast<float>(textArea.getCentreY() - 3), 6.0f, 6.0f);
        textArea.removeFromLeft(12);
    }
    graphics.drawFittedText(text, textArea, juce::Justification::centredLeft, 1);
}

void LumenLookAndFeel::drawToggleButton(
    juce::Graphics& graphics,
    juce::ToggleButton& button,
    bool shouldDrawButtonAsHighlighted,
    bool)
{
    const auto bounds = button.getLocalBounds().toFloat();
    const float trackWidth = 34.0f;
    const float trackHeight = 18.0f;
    const auto track = juce::Rectangle<float>(bounds.getX(), bounds.getCentreY() - trackHeight * 0.5f, trackWidth, trackHeight);
    const bool isOn = button.getToggleState();

    auto trackColour = isOn ? design::accentDark() : design::bgElevated();
    if (shouldDrawButtonAsHighlighted)
        trackColour = trackColour.brighter(0.08f);

    graphics.setColour(trackColour);
    graphics.fillRoundedRectangle(track, trackHeight * 0.5f);
    graphics.setColour(design::borderStrong());
    graphics.drawRoundedRectangle(track, trackHeight * 0.5f, 1.0f);

    const float thumb = 14.0f;
    const float travel = trackWidth - thumb - 4.0f;
    const float thumbX = track.getX() + 2.0f + (isOn ? travel : 0.0f);
    graphics.setColour(isOn ? design::accent() : design::textSecondary());
    graphics.fillEllipse(thumbX, track.getCentreY() - thumb * 0.5f, thumb, thumb);

    graphics.setColour(button.isEnabled() ? design::textSecondary() : design::textMuted());
    graphics.setFont(design::microFont());
    graphics.drawFittedText(
        button.getButtonText(),
        button.getLocalBounds().withTrimmedLeft(static_cast<int>(trackWidth) + 8),
        juce::Justification::centredLeft,
        1);
}

void LumenLookAndFeel::drawLabel(juce::Graphics& graphics, juce::Label& label)
{
    graphics.setColour(label.findColour(juce::Label::textColourId));
    graphics.setFont(getLabelFont(label));
    graphics.drawFittedText(label.getText(), label.getLocalBounds(), label.getJustificationType(), 2);
}

juce::Font LumenLookAndFeel::getTextButtonFont(juce::TextButton&, int) { return design::bodyFont(); }
juce::Font LumenLookAndFeel::getComboBoxFont(juce::ComboBox&) { return design::bodyFont(); }
juce::Font LumenLookAndFeel::getLabelFont(juce::Label&) { return design::bodyFont(); }
juce::Font LumenLookAndFeel::getPopupMenuFont() { return design::bodyFont(); }

} // namespace lumen::ui
