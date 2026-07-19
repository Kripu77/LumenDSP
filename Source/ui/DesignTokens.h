#pragma once

#include <JuceHeader.h>

namespace lumen::design
{

inline constexpr int spacingUnitPixels = 8;
inline constexpr int spacingHalfUnitPixels = 4;
inline constexpr int spacingTwoUnitsPixels = 16;
inline constexpr int spacingThreeUnitsPixels = 24;
inline constexpr int spacingFourUnitsPixels = 32;

inline constexpr int windowWidthPixels = 1320;
inline constexpr int windowHeightPixels = 920;
inline constexpr int windowMinimumWidthPixels = 1040;
inline constexpr int windowMinimumHeightPixels = 720;

inline constexpr int topChromeHeightPixels = 96;
inline constexpr int signalPathHeightPixels = 88;
inline constexpr int bottomStatusHeightPixels = 28;

inline constexpr int panelCornerRadiusPixels = 10;
inline constexpr int controlCornerRadiusPixels = 8;
inline constexpr int pedalCornerRadiusPixels = 14;
inline constexpr int knobSizePixels = 52;
inline constexpr int chromeKnobSizePixels = 44;
inline constexpr int knobValueArcThicknessPixels = 3;
inline constexpr int knobPointerLengthPixels = 10;
inline constexpr int knobPointerThicknessPixels = 2;
inline constexpr float knobStartAngleRadians = juce::MathConstants<float>::pi * 1.25f;
inline constexpr float knobEndAngleRadians = juce::MathConstants<float>::pi * 2.75f;
inline constexpr float knobDragSensitivityPixelsPerFullRange = 170.0f;

inline constexpr int meterSegmentCount = 22;
inline constexpr int meterWidthPixels = 10;
inline constexpr int meterSegmentGapPixels = 2;
inline constexpr float sectionActiveIndicatorHeightPixels = 3.0f;
inline constexpr float meterPeakHoldSeconds = 1.2f;
inline constexpr float meterDecayPerSecond = 8.0f;
inline constexpr float meterFloorDb = -60.0f;
inline constexpr float meterCeilingDb = 0.0f;
inline constexpr float meterWarningThresholdDb = -6.0f;
inline constexpr float meterClipThresholdDb = -0.5f;

inline constexpr int titleFontHeightPoints = 16;
inline constexpr int brandFontHeightPoints = 15;
inline constexpr int sectionFontHeightPoints = 11;
inline constexpr int bodyFontHeightPoints = 12;
inline constexpr int valueFontHeightPoints = 10;
inline constexpr int microFontHeightPoints = 9;
inline constexpr int hardwareLabelFontHeightPoints = 10;
inline constexpr int hardwareBrandFontHeightPoints = 14;

inline constexpr float controlHoverBrightnessLift = 0.06f;
inline constexpr double uiTimerIntervalHertz = 30.0;

inline juce::Colour bgPrimary() { return juce::Colour::fromRGB(7, 9, 14); }
inline juce::Colour bgSecondary() { return juce::Colour::fromRGB(13, 17, 24); }
inline juce::Colour bgTertiary() { return juce::Colour::fromRGB(19, 24, 32); }
inline juce::Colour bgSurface() { return juce::Colour::fromRGB(20, 28, 38); }
inline juce::Colour bgElevated() { return juce::Colour::fromRGB(26, 34, 48); }
inline juce::Colour bgCard() { return juce::Colour::fromRGB(17, 24, 32); }
inline juce::Colour bgHeader() { return juce::Colour::fromRGB(10, 14, 20); }

inline juce::Colour borderLight() { return juce::Colour::fromRGB(42, 53, 72); }
inline juce::Colour borderMedium() { return juce::Colour::fromRGB(36, 48, 64); }
inline juce::Colour borderStrong() { return juce::Colour::fromRGB(61, 77, 102); }

inline juce::Colour textPrimary() { return juce::Colour::fromRGB(238, 243, 250); }
inline juce::Colour textSecondary() { return juce::Colour::fromRGB(168, 180, 200); }
inline juce::Colour textTertiary() { return juce::Colour::fromRGB(140, 155, 175); }
inline juce::Colour textMuted() { return juce::Colour::fromRGB(110, 125, 146); }

inline juce::Colour accent() { return juce::Colour::fromRGB(94, 200, 255); }
inline juce::Colour accentHover() { return juce::Colour::fromRGB(138, 216, 255); }
inline juce::Colour accentDark() { return juce::Colour::fromRGB(58, 160, 216); }
inline juce::Colour accentSoft() { return accent().withAlpha(0.18f); }

inline juce::Colour success() { return juce::Colour::fromRGB(46, 200, 104); }
inline juce::Colour successDim() { return juce::Colour::fromRGB(32, 140, 72); }

inline juce::Colour knobRing() { return success(); }
inline juce::Colour knobRingTrack() { return juce::Colours::white.withAlpha(0.08f); }
inline juce::Colour knobBodyFrom() { return juce::Colour::fromRGB(69, 69, 88); }
inline juce::Colour knobBodyTo() { return juce::Colour::fromRGB(24, 24, 30); }
inline juce::Colour knobIndicator() { return juce::Colour::fromRGB(220, 220, 230); }

inline juce::Colour meterNormal() { return success(); }
inline juce::Colour meterWarning() { return juce::Colour::fromRGB(242, 184, 72); }
inline juce::Colour meterClip() { return juce::Colour::fromRGB(220, 96, 96); }
inline juce::Colour meterInactive() { return juce::Colour::fromRGB(48, 48, 60); }

inline juce::Colour nodeInput() { return juce::Colour::fromRGB(104, 168, 104); }
inline juce::Colour nodeAmp() { return accent(); }
inline juce::Colour nodeEq() { return juce::Colour::fromRGB(104, 136, 184); }
inline juce::Colour nodeCab() { return juce::Colour::fromRGB(168, 136, 104); }

inline juce::Colour ledWarm() { return accent(); }
inline juce::Colour ledWarmDim() { return accentDark(); }
inline juce::Colour ledCool() { return success(); }

inline juce::Colour chromeSurface() { return bgSecondary(); }
inline juce::Colour chromeSurfaceRaised() { return bgElevated(); }
inline juce::Colour chromeBorder() { return borderLight(); }
inline juce::Colour chromeTextPrimary() { return textPrimary(); }
inline juce::Colour chromeTextSecondary() { return textSecondary(); }
inline juce::Colour chromeTextMuted() { return textMuted(); }

inline juce::Colour metalDeep() { return bgPrimary(); }
inline juce::Colour metalPanel() { return bgCard(); }
inline juce::Colour metalRaised() { return bgElevated(); }
inline juce::Colour metalBorder() { return borderStrong(); }
inline juce::Colour metalTextPrimary() { return textPrimary(); }
inline juce::Colour metalTextSecondary() { return textSecondary(); }
inline juce::Colour metalTextMuted() { return textMuted(); }
inline juce::Colour studioBackdropTop() { return bgPrimary(); }
inline juce::Colour studioBackdropBottom() { return juce::Colour::fromRGB(12, 12, 16); }

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
    graphics.setColour(bgPrimary());
    graphics.fillRect(bounds);
}

inline void drawSoftShadow(juce::Graphics& graphics, juce::Rectangle<float> bounds, float cornerRadius, float opacity = 0.45f)
{
    for (int layer = 3; layer >= 1; --layer)
    {
        const float expand = static_cast<float>(layer) * 2.5f;
        graphics.setColour(juce::Colours::black.withAlpha(opacity * (0.12f / static_cast<float>(layer))));
        graphics.fillRoundedRectangle(bounds.expanded(expand).translated(0.0f, expand * 0.4f), cornerRadius + expand * 0.2f);
    }
}

inline void drawCard(juce::Graphics& graphics, juce::Rectangle<float> bounds, float cornerRadius = 10.0f)
{
    drawSoftShadow(graphics, bounds, cornerRadius, 0.5f);
    graphics.setColour(bgCard());
    graphics.fillRoundedRectangle(bounds, cornerRadius);
    graphics.setColour(borderLight());
    graphics.drawRoundedRectangle(bounds, cornerRadius, 1.0f);
}

inline void drawMetalPanel(juce::Graphics& graphics, juce::Rectangle<float> bounds, float cornerRadius)
{
    drawCard(graphics, bounds, cornerRadius);
}

inline void drawChromePanel(juce::Graphics& graphics, juce::Rectangle<float> bounds, float cornerRadius)
{
    drawCard(graphics, bounds, cornerRadius);
}

} // namespace lumen::design
