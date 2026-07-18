#pragma once

#include <JuceHeader.h>
#include "ui/DesignTokens.h"

namespace lumen::ui
{

class LumenLookAndFeel : public juce::LookAndFeel_V4
{
public:
    LumenLookAndFeel();

    void drawRotarySlider(
        juce::Graphics& graphics,
        int x,
        int y,
        int width,
        int height,
        float sliderPosProportional,
        float rotaryStartAngle,
        float rotaryEndAngle,
        juce::Slider& slider) override;

    void drawLinearSlider(
        juce::Graphics& graphics,
        int x,
        int y,
        int width,
        int height,
        float sliderPos,
        float minSliderPos,
        float maxSliderPos,
        juce::Slider::SliderStyle style,
        juce::Slider& slider) override;

    void drawButtonBackground(
        juce::Graphics& graphics,
        juce::Button& button,
        const juce::Colour& backgroundColour,
        bool shouldDrawButtonAsHighlighted,
        bool shouldDrawButtonAsDown) override;

    void drawButtonText(
        juce::Graphics& graphics,
        juce::TextButton& button,
        bool shouldDrawButtonAsHighlighted,
        bool shouldDrawButtonAsDown) override;

    void drawComboBox(
        juce::Graphics& graphics,
        int width,
        int height,
        bool isButtonDown,
        int buttonX,
        int buttonY,
        int buttonW,
        int buttonH,
        juce::ComboBox& comboBox) override;

    void drawPopupMenuBackground(juce::Graphics& graphics, int width, int height) override;

    void drawPopupMenuItem(
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
        const juce::Colour* textColour) override;

    void drawToggleButton(
        juce::Graphics& graphics,
        juce::ToggleButton& button,
        bool shouldDrawButtonAsHighlighted,
        bool shouldDrawButtonAsDown) override;

    void drawLabel(juce::Graphics& graphics, juce::Label& label) override;

    juce::Font getTextButtonFont(juce::TextButton& button, int buttonHeight) override;
    juce::Font getComboBoxFont(juce::ComboBox& comboBox) override;
    juce::Font getLabelFont(juce::Label& label) override;
    juce::Font getPopupMenuFont() override;

private:
    void applyColourScheme();
    void drawChromeRotary(
        juce::Graphics& graphics,
        juce::Rectangle<float> bounds,
        float sliderPosProportional,
        float rotaryStartAngle,
        float rotaryEndAngle);
    void drawMetalRotary(
        juce::Graphics& graphics,
        juce::Rectangle<float> bounds,
        float sliderPosProportional,
        float rotaryStartAngle,
        float rotaryEndAngle);
};

} // namespace lumen::ui
