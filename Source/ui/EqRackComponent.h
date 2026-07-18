#pragma once

#include <JuceHeader.h>
#include "ui/DesignTokens.h"
#include "ui/KnobComponent.h"

namespace lumen::ui
{

class EqRackComponent : public juce::Component
{
public:
    EqRackComponent();

    void paint(juce::Graphics& graphics) override;
    void resized() override;

    juce::Slider& getBassSlider() noexcept;
    juce::Slider& getMidSlider() noexcept;
    juce::Slider& getTrebleSlider() noexcept;
    juce::ToggleButton& getEnableButton() noexcept;

private:
    void configureSlider(juce::Slider& slider, juce::Label& label, const juce::String& text);

    juce::Slider bassSlider;
    juce::Slider midSlider;
    juce::Slider trebleSlider;
    juce::Label bassLabel;
    juce::Label midLabel;
    juce::Label trebleLabel;
    juce::Label titleLabel;
    juce::Label bandHintLabel;
    juce::ToggleButton enableButton{"POWER"};
};

} // namespace lumen::ui
