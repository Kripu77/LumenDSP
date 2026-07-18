#include "ui/LedMeterComponent.h"

namespace lumen::ui
{

LedMeterComponent::LedMeterComponent(const juce::String& meterLabel, Theme theme)
    : labelText(meterLabel)
    , visualTheme(theme)
{
}

void LedMeterComponent::paint(juce::Graphics& graphics)
{
    auto bounds = getLocalBounds().toFloat();
    auto labelArea = bounds.removeFromBottom(static_cast<float>(design::spacingTwoUnitsPixels));
    auto meterArea = bounds.reduced(1.5f, 0.0f);

    graphics.setColour(
        visualTheme == Theme::chrome ? design::chromeTextMuted() : design::metalTextMuted());
    graphics.setFont(design::microFont());
    graphics.drawText(labelText, labelArea, juce::Justification::centred);

    const float segmentGap = static_cast<float>(design::meterSegmentGapPixels);
    const float segmentHeight =
        (meterArea.getHeight() - segmentGap * static_cast<float>(design::meterSegmentCount - 1))
        / static_cast<float>(design::meterSegmentCount);
    const float normalisedPeak = normaliseDb(peakLevelDb);
    const float normalisedHold = normaliseDb(peakHoldLevelDb);
    const int activeSegmentCount = juce::jlimit(
        0,
        design::meterSegmentCount,
        static_cast<int>(std::round(normalisedPeak * static_cast<float>(design::meterSegmentCount))));
    const int holdSegmentIndex = juce::jlimit(
        0,
        design::meterSegmentCount - 1,
        static_cast<int>(std::round(normalisedHold * static_cast<float>(design::meterSegmentCount))) - 1);

    for (int segmentIndex = 0; segmentIndex < design::meterSegmentCount; ++segmentIndex)
    {
        const int visualIndexFromTop = design::meterSegmentCount - 1 - segmentIndex;
        const float segmentY = meterArea.getY() + static_cast<float>(visualIndexFromTop) * (segmentHeight + segmentGap);
        const auto segmentBounds = juce::Rectangle<float>(
            meterArea.getX(),
            segmentY,
            meterArea.getWidth(),
            segmentHeight);

        const bool isActive = segmentIndex < activeSegmentCount;
        const bool isHold = segmentIndex == holdSegmentIndex;

        if (isActive)
            graphics.setColour(colourForSegment(segmentIndex));
        else if (isHold)
            graphics.setColour(
                visualTheme == Theme::chrome ? design::chromeTextSecondary().withAlpha(0.85f)
                                             : design::metalTextSecondary().withAlpha(0.85f));
        else
            graphics.setColour(
                visualTheme == Theme::chrome ? design::meterInactive() : design::meterInactiveDark());

        graphics.fillRoundedRectangle(segmentBounds, 1.4f);
    }
}

void LedMeterComponent::setLevels(float peakDb, float peakHoldDb)
{
    peakLevelDb = peakDb;
    peakHoldLevelDb = peakHoldDb;
    repaint();
}

void LedMeterComponent::setTheme(Theme theme)
{
    visualTheme = theme;
    repaint();
}

float LedMeterComponent::normaliseDb(float levelDb) const
{
    const float clamped = juce::jlimit(design::meterFloorDb, design::meterCeilingDb, levelDb);
    return (clamped - design::meterFloorDb) / (design::meterCeilingDb - design::meterFloorDb);
}

juce::Colour LedMeterComponent::colourForSegment(int segmentIndex) const
{
    const float segmentTopDb =
        design::meterFloorDb
        + (static_cast<float>(segmentIndex + 1) / static_cast<float>(design::meterSegmentCount))
              * (design::meterCeilingDb - design::meterFloorDb);

    if (segmentTopDb >= design::meterClipThresholdDb)
        return design::meterClip();
    if (segmentTopDb >= design::meterWarningThresholdDb)
        return design::meterWarning();
    return design::meterNormal();
}

} // namespace lumen::ui
