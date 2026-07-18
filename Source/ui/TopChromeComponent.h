#pragma once

#include <JuceHeader.h>
#include "ui/DesignTokens.h"
#include "ui/KnobComponent.h"
#include "ui/LedMeterComponent.h"
#include "ui/PresetBarComponent.h"

namespace lumen::ui
{

class TopChromeComponent : public juce::Component
{
public:
    TopChromeComponent();

    void paint(juce::Graphics& graphics) override;
    void resized() override;

    KnobComponent& getInputKnob() noexcept;
    KnobComponent& getGateKnob() noexcept;
    KnobComponent& getOutputKnob() noexcept;
    juce::ToggleButton& getGateEnableButton() noexcept;
    LedMeterComponent& getInputMeter() noexcept;
    LedMeterComponent& getOutputMeter() noexcept;
    PresetBarComponent& getPresetBar() noexcept;
    juce::TextButton& getAudioButton() noexcept;

private:
    juce::Label brandLabel;
    juce::Label productLabel;
    LedMeterComponent inputMeter{"IN", LedMeterComponent::Theme::chrome};
    LedMeterComponent outputMeter{"OUT", LedMeterComponent::Theme::chrome};
    KnobComponent inputKnob{"INPUT", "dB", KnobComponent::Style::chrome};
    KnobComponent gateKnob{"GATE", "dB", KnobComponent::Style::chrome};
    KnobComponent outputKnob{"OUTPUT", "dB", KnobComponent::Style::chrome};
    juce::ToggleButton gateEnableButton{"GATE"};
    PresetBarComponent presetBar;
    juce::TextButton audioButton{"AUDIO"};
};

} // namespace lumen::ui
