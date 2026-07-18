#pragma once

#include <JuceHeader.h>
#include "ui/DesignTokens.h"

namespace lumen::ui
{

class LedMeterComponent : public juce::Component
{
public:
    enum class Theme
    {
        chrome,
        dark
    };

    explicit LedMeterComponent(const juce::String& meterLabel, Theme theme = Theme::chrome);

    void paint(juce::Graphics& graphics) override;
    void setLevels(float peakDb, float peakHoldDb);
    void setTheme(Theme theme);

private:
    float normaliseDb(float levelDb) const;
    juce::Colour colourForSegment(int segmentIndex) const;

    juce::String labelText;
    Theme visualTheme;
    float peakLevelDb = design::meterFloorDb;
    float peakHoldLevelDb = design::meterFloorDb;
};

} // namespace lumen::ui
