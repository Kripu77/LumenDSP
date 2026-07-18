#pragma once

#include <JuceHeader.h>
#include "ui/DesignTokens.h"

namespace lumen::ui
{

class LedMeterComponent : public juce::Component
{
public:
    explicit LedMeterComponent(const juce::String& meterLabel);

    void paint(juce::Graphics& graphics) override;
    void resized() override;
    void setLevels(float peakDb, float peakHoldDb);

private:
    float normaliseDb(float levelDb) const;
    juce::Colour colourForSegment(int segmentIndex, int activeSegmentCount) const;

    juce::String labelText;
    float peakLevelDb = design::meterFloorDb;
    float peakHoldLevelDb = design::meterFloorDb;
};

} // namespace lumen::ui
