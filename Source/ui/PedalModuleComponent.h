#pragma once

#include <JuceHeader.h>
#include "ui/DesignTokens.h"
#include "ui/KnobComponent.h"

namespace lumen::ui
{

class PedalModuleComponent : public juce::Component
{
public:
    enum class Finish
    {
        steel,
        olive,
        charcoal
    };

    PedalModuleComponent(
        const juce::String& titleText,
        const juce::String& subtitleText,
        Finish finish);

    void paint(juce::Graphics& graphics) override;
    void resized() override;

    KnobComponent& getPrimaryKnob() noexcept;
    juce::ToggleButton& getEnableButton() noexcept;
    void setEngaged(bool isEngaged);

private:
    juce::Colour finishColour() const;

    juce::String title;
    juce::String subtitle;
    Finish pedalFinish;
    bool engaged = true;
    KnobComponent primaryKnob;
    juce::ToggleButton enableButton{"ENGAGE"};
    juce::Label titleLabel;
    juce::Label subtitleLabel;
};

} // namespace lumen::ui
