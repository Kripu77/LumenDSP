#pragma once

#include <JuceHeader.h>

namespace lumen::design
{

inline constexpr int spacingUnitPixels = 8;
inline constexpr int spacingHalfUnitPixels = spacingUnitPixels / 2;
inline constexpr int spacingTwoUnitsPixels = spacingUnitPixels * 2;
inline constexpr int spacingThreeUnitsPixels = spacingUnitPixels * 3;
inline constexpr int spacingFourUnitsPixels = spacingUnitPixels * 4;
inline constexpr int spacingSixUnitsPixels = spacingUnitPixels * 6;

inline constexpr int windowWidthPixels = 960;
inline constexpr int windowHeightPixels = 560;
inline constexpr int windowMinimumWidthPixels = 800;
inline constexpr int windowMinimumHeightPixels = 480;

inline constexpr int panelCornerRadiusPixels = 12;
inline constexpr int controlCornerRadiusPixels = 8;
inline constexpr int knobSizePixels = 72;
inline constexpr int knobValueArcThicknessPixels = 3;
inline constexpr int knobPointerLengthPixels = 18;
inline constexpr int knobPointerThicknessPixels = 2;
inline constexpr float knobStartAngleRadians = juce::MathConstants<float>::pi * 1.25f;
inline constexpr float knobEndAngleRadians = juce::MathConstants<float>::pi * 2.75f;
inline constexpr float knobDragSensitivityPixelsPerFullRange = 200.0f;

inline constexpr int meterSegmentCount = 24;
inline constexpr int meterWidthPixels = 14;
inline constexpr int meterHeightPixels = 220;
inline constexpr int meterSegmentGapPixels = 2;
inline constexpr float meterPeakHoldSeconds = 1.2f;
inline constexpr float meterDecayPerSecond = 8.0f;
inline constexpr float meterFloorDb = -60.0f;
inline constexpr float meterCeilingDb = 0.0f;
inline constexpr float meterWarningThresholdDb = -6.0f;
inline constexpr float meterClipThresholdDb = -0.5f;

inline constexpr int signalFlowNodeHeightPixels = 36;
inline constexpr int signalFlowConnectorWidthPixels = 28;
inline constexpr float signalFlowActiveGlowAlpha = 0.35f;

inline constexpr int titleFontHeightPoints = 22;
inline constexpr int sectionFontHeightPoints = 13;
inline constexpr int bodyFontHeightPoints = 12;
inline constexpr int valueFontHeightPoints = 11;
inline constexpr int microFontHeightPoints = 10;

inline constexpr float panelBackgroundAlpha = 0.92f;
inline constexpr float controlHoverBrightnessLift = 0.08f;
inline constexpr float animationDurationSeconds = 0.18f;
inline constexpr float presetRecallAnimationSeconds = 0.28f;
inline constexpr double uiTimerIntervalHertz = 30.0;

inline juce::Colour backgroundDeep()
{
    return juce::Colour::fromRGB(12, 14, 18);
}

inline juce::Colour backgroundPanel()
{
    return juce::Colour::fromRGB(22, 26, 34);
}

inline juce::Colour backgroundElevated()
{
    return juce::Colour::fromRGB(32, 38, 48);
}

inline juce::Colour backgroundControl()
{
    return juce::Colour::fromRGB(40, 46, 58);
}

inline juce::Colour borderSubtle()
{
    return juce::Colour::fromRGB(58, 66, 82);
}

inline juce::Colour textPrimary()
{
    return juce::Colour::fromRGB(236, 240, 248);
}

inline juce::Colour textSecondary()
{
    return juce::Colour::fromRGB(156, 166, 184);
}

inline juce::Colour textMuted()
{
    return juce::Colour::fromRGB(110, 120, 138);
}

inline juce::Colour accent()
{
    return juce::Colour::fromRGB(56, 214, 232);
}

inline juce::Colour accentDim()
{
    return juce::Colour::fromRGB(28, 120, 132);
}

inline juce::Colour accentSoft()
{
    return accent().withAlpha(0.18f);
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
    return juce::Colour::fromRGB(48, 54, 66);
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

} // namespace lumen::design
