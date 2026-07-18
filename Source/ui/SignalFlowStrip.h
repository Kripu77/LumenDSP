#pragma once

#include <JuceHeader.h>
#include "ui/DesignTokens.h"

namespace lumen::ui
{

class SignalFlowStrip : public juce::Component
{
public:
    enum class Stage
    {
        input = 0,
        amp,
        eq,
        cab,
        output,
        count
    };

    SignalFlowStrip();

    void paint(juce::Graphics& graphics) override;
    void resized() override;
    void setStageActive(Stage stage, bool isActive);
    void setStageLabel(Stage stage, const juce::String& label);

private:
    struct StageVisual
    {
        juce::String label;
        bool active = false;
        juce::Rectangle<float> bounds;
    };

    void layoutStages();
    void drawStage(juce::Graphics& graphics, const StageVisual& stageVisual) const;
    void drawConnector(
        juce::Graphics& graphics,
        const juce::Rectangle<float>& fromBounds,
        const juce::Rectangle<float>& toBounds,
        bool active) const;

    std::array<StageVisual, static_cast<size_t>(Stage::count)> stages{};
};

} // namespace lumen::ui
