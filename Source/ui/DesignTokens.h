#pragma once

#include <JuceHeader.h>

namespace lumen::design
{

inline constexpr int spacingUnitPixels = 8;
inline constexpr int spacingHalfUnitPixels = spacingUnitPixels / 2;
inline constexpr int spacingTwoUnitsPixels = spacingUnitPixels * 2;
inline constexpr int spacingThreeUnitsPixels = spacingUnitPixels * 3;
inline constexpr int spacingFourUnitsPixels = spacingUnitPixels * 4;
inline constexpr int spacingFiveUnitsPixels = spacingUnitPixels * 5;
inline constexpr int spacingSixUnitsPixels = spacingUnitPixels * 6;
inline constexpr int spacingEightUnitsPixels = spacingUnitPixels * 8;

inline constexpr int windowWidthPixels = 1120;
inline constexpr int windowHeightPixels = 720;
inline constexpr int windowMinimumWidthPixels = 960;
inline constexpr int windowMinimumHeightPixels = 640;

inline constexpr int topChromeHeightPixels = 112;
inline constexpr int bottomNavHeightPixels = 72;
inline constexpr int chromeControlSizePixels = 56;
inline constexpr int chromeMeterWidthPixels = 10;
inline constexpr int chromeMeterHeightPixels = 72;

inline constexpr int panelCornerRadiusPixels = 16;
inline constexpr int controlCornerRadiusPixels = 10;
inline constexpr int pedalCornerRadiusPixels = 14;
inline constexpr int knobSizePixels = 68;
inline constexpr int faceplateKnobSizePixels = 64;
inline constexpr int chromeKnobSizePixels = 52;
inline constexpr int knobValueArcThicknessPixels = 3;
inline constexpr int knobPointerLengthPixels = 16;
inline constexpr int knobPointerThicknessPixels = 2;
inline constexpr float knobStartAngleRadians = juce::MathConstants<float>::pi * 1.25f;
inline constexpr float knobEndAngleRadians = juce::MathConstants<float>::pi * 2.75f;
inline constexpr float knobDragSensitivityPixelsPerFullRange = 180.0f;

inline constexpr int meterSegmentCount = 28;
inline constexpr int meterWidthPixels = 12;
inline constexpr int meterHeightPixels = 200;
inline constexpr int meterSegmentGapPixels = 2;
inline constexpr float meterPeakHoldSeconds = 1.2f;
inline constexpr float meterDecayPerSecond = 8.0f;
inline constexpr float meterFloorDb = -60.0f;
inline constexpr float meterCeilingDb = 0.0f;
inline constexpr float meterWarningThresholdDb = -6.0f;
inline constexpr float meterClipThresholdDb = -0.5f;

inline constexpr int sectionNavIconSizePixels = 36;
inline constexpr int sectionNavItemWidthPixels = 72;
inline constexpr float sectionActiveIndicatorHeightPixels = 3.0f;

inline constexpr int titleFontHeightPoints = 18;
inline constexpr int brandFontHeightPoints = 20;
inline constexpr int sectionFontHeightPoints = 12;
inline constexpr int bodyFontHeightPoints = 12;
inline constexpr int valueFontHeightPoints = 11;
inline constexpr int microFontHeightPoints = 10;
inline constexpr int hardwareLabelFontHeightPoints = 11;
inline constexpr int hardwareBrandFontHeightPoints = 16;

inline constexpr float panelBackgroundAlpha = 1.0f;
inline constexpr float controlHoverBrightnessLift = 0.06f;
inline constexpr float animationDurationSeconds = 0.22f;
inline constexpr float stageFadeDurationSeconds = 0.16f;
inline constexpr double uiTimerIntervalHertz = 30.0;

inline constexpr float shadowOpacity = 0.18f;
inline constexpr float shadowRadiusPixels = 24.0f;
inline constexpr float hardwareBevelHighlightAlpha = 0.14f;
inline constexpr float hardwareBevelShadowAlpha = 0.35f;
inline constexpr float ledGlowAlpha = 0.55f;

inline juce::Colour studioBackdropTop()
{
    return juce::Colour::fromRGB(198, 206, 216);
}

inline juce::Colour studioBackdropBottom()
{
    return juce::Colour::fromRGB(168, 178, 192);
}

inline juce::Colour chromeSurface()
{
    return juce::Colour::fromRGB(236, 240, 245);
}

inline juce::Colour chromeSurfaceRaised()
{
    return juce::Colour::fromRGB(248, 250, 252);
}

inline juce::Colour chromeBorder()
{
    return juce::Colour::fromRGB(196, 204, 214);
}

inline juce::Colour chromeTextPrimary()
{
    return juce::Colour::fromRGB(42, 48, 58);
}

inline juce::Colour chromeTextSecondary()
{
    return juce::Colour::fromRGB(96, 106, 120);
}

inline juce::Colour chromeTextMuted()
{
    return juce::Colour::fromRGB(132, 142, 156);
}

inline juce::Colour metalDeep()
{
    return juce::Colour::fromRGB(28, 31, 36);
}

inline juce::Colour metalPanel()
{
    return juce::Colour::fromRGB(38, 42, 48);
}

inline juce::Colour metalRaised()
{
    return juce::Colour::fromRGB(52, 57, 64);
}

inline juce::Colour metalBorder()
{
    return juce::Colour::fromRGB(72, 78, 88);
}

inline juce::Colour metalTextPrimary()
{
    return juce::Colour::fromRGB(236, 240, 244);
}

inline juce::Colour metalTextSecondary()
{
    return juce::Colour::fromRGB(164, 172, 184);
}

inline juce::Colour metalTextMuted()
{
    return juce::Colour::fromRGB(118, 126, 138);
}

inline juce::Colour accent()
{
    return juce::Colour::fromRGB(56, 168, 255);
}

inline juce::Colour accentDim()
{
    return juce::Colour::fromRGB(32, 96, 160);
}

inline juce::Colour accentSoft()
{
    return accent().withAlpha(0.16f);
}

inline juce::Colour ledWarm()
{
    return juce::Colour::fromRGB(255, 120, 48);
}

inline juce::Colour ledWarmDim()
{
    return juce::Colour::fromRGB(140, 64, 28);
}

inline juce::Colour ledCool()
{
    return juce::Colour::fromRGB(72, 196, 255);
}

inline juce::Colour meterNormal()
{
    return accent();
}

inline juce::Colour meterWarning()
{
    return juce::Colour::fromRGB(242, 184, 72);
}

inline juce::Colour meterClip()
{
    return juce::Colour::fromRGB(240, 88, 96);
}

inline juce::Colour meterInactive()
{
    return juce::Colour::fromRGB(210, 216, 224);
}

inline juce::Colour meterInactiveDark()
{
    return juce::Colour::fromRGB(48, 54, 62);
}

inline juce::Font brandFont()
{
    return juce::Font(juce::FontOptions(static_cast<float>(brandFontHeightPoints)).withStyle("Bold"));
}

inline juce::Font titleFont()
{
    return juce::Font(juce::FontOptions(static_cast<float>(titleFontHeightPoints)).withStyle("Bold"));
}

inline juce::Font sectionFont()
{
    return juce::Font(juce::FontOptions(static_cast<float>(sectionFontHeightPoints)).withStyle("Bold"));
}

inline juce::Font bodyFont()
{
    return juce::Font(juce::FontOptions(static_cast<float>(bodyFontHeightPoints)));
}

inline juce::Font valueFont()
{
    return juce::Font(juce::FontOptions(static_cast<float>(valueFontHeightPoints)));
}

inline juce::Font microFont()
{
    return juce::Font(juce::FontOptions(static_cast<float>(microFontHeightPoints)));
}

inline juce::Font hardwareLabelFont()
{
    return juce::Font(juce::FontOptions(static_cast<float>(hardwareLabelFontHeightPoints)).withStyle("Bold"));
}

inline juce::Font hardwareBrandFont()
{
    return juce::Font(juce::FontOptions(static_cast<float>(hardwareBrandFontHeightPoints)).withStyle("Bold"));
}

inline void fillStudioBackdrop(juce::Graphics& graphics, juce::Rectangle<float> bounds)
{
    juce::ColourGradient gradient(
        studioBackdropTop(),
        bounds.getCentreX(),
        bounds.getY(),
        studioBackdropBottom(),
        bounds.getCentreX(),
        bounds.getBottom(),
        false);
    graphics.setGradientFill(gradient);
    graphics.fillRect(bounds);
}

inline void drawSoftShadow(
    juce::Graphics& graphics,
    juce::Rectangle<float> bounds,
    float cornerRadius,
    float opacity = shadowOpacity)
{
    for (int layerIndex = 4; layerIndex >= 1; --layerIndex)
    {
        const float expand = static_cast<float>(layerIndex) * 3.0f;
        const float alpha = opacity * (0.08f / static_cast<float>(layerIndex));
        graphics.setColour(juce::Colours::black.withAlpha(alpha));
        graphics.fillRoundedRectangle(bounds.expanded(expand).translated(0.0f, expand * 0.35f), cornerRadius + expand * 0.25f);
    }
}

inline void drawMetalPanel(
    juce::Graphics& graphics,
    juce::Rectangle<float> bounds,
    float cornerRadius)
{
    drawSoftShadow(graphics, bounds, cornerRadius, 0.22f);

    juce::ColourGradient body(
        metalRaised(),
        bounds.getX(),
        bounds.getY(),
        metalDeep(),
        bounds.getX(),
        bounds.getBottom(),
        false);
    graphics.setGradientFill(body);
    graphics.fillRoundedRectangle(bounds, cornerRadius);

    graphics.setColour(juce::Colours::white.withAlpha(hardwareBevelHighlightAlpha));
    graphics.drawRoundedRectangle(bounds.reduced(1.0f), cornerRadius - 1.0f, 1.0f);

    graphics.setColour(juce::Colours::black.withAlpha(hardwareBevelShadowAlpha));
    graphics.drawRoundedRectangle(bounds, cornerRadius, 1.2f);
}

inline void drawChromePanel(
    juce::Graphics& graphics,
    juce::Rectangle<float> bounds,
    float cornerRadius)
{
    drawSoftShadow(graphics, bounds, cornerRadius, 0.12f);
    graphics.setColour(chromeSurfaceRaised());
    graphics.fillRoundedRectangle(bounds, cornerRadius);
    graphics.setColour(chromeBorder());
    graphics.drawRoundedRectangle(bounds, cornerRadius, 1.0f);
}

} // namespace lumen::design
