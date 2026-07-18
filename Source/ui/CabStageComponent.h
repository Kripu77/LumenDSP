#pragma once

#include <JuceHeader.h>
#include "ui/DesignTokens.h"
#include "ui/FileSlotComponent.h"

namespace lumen::ui
{

class CabStageComponent : public juce::Component
{
public:
    CabStageComponent();

    void paint(juce::Graphics& graphics) override;
    void resized() override;

    FileSlotComponent& getIrSlot() noexcept;
    juce::ToggleButton& getEnableButton() noexcept;
    void setIrStatus(const juce::String& irName, bool isLoaded);

private:
    FileSlotComponent irSlot{"CABINET IR", "*.wav"};
    juce::ToggleButton enableButton{"CABINET"};
    juce::Label titleLabel;
    juce::Label statusLabel;
    juce::Label footerLabel;
};

} // namespace lumen::ui
