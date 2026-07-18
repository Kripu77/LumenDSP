#pragma once

#include <JuceHeader.h>
#include "ui/DesignTokens.h"
#include "ui/FileSlotComponent.h"
#include "ui/KnobComponent.h"

namespace lumen::ui
{

class AmpFaceplateComponent : public juce::Component
{
public:
    AmpFaceplateComponent();

    void paint(juce::Graphics& graphics) override;
    void resized() override;

    FileSlotComponent& getModelSlot() noexcept;
    KnobComponent& getBassKnob() noexcept;
    KnobComponent& getMidKnob() noexcept;
    KnobComponent& getTrebleKnob() noexcept;
    juce::ToggleButton& getEqEnableButton() noexcept;

    void setModelStatus(const juce::String& modelName, bool isLoaded, bool isLoading);

private:
    void paintTubeGlow(juce::Graphics& graphics, juce::Rectangle<float> grilleBounds) const;

    FileSlotComponent modelSlot{"NAM MODEL", "*.nam"};
    KnobComponent bassKnob{"BASS", "dB", KnobComponent::Style::metal};
    KnobComponent midKnob{"MID", "dB", KnobComponent::Style::metal};
    KnobComponent trebleKnob{"TREBLE", "dB", KnobComponent::Style::metal};
    juce::ToggleButton eqEnableButton{"EQ INTO AMP PATH"};
    juce::Label chassisLabel;
    juce::Label modelStatusLabel;
    juce::Label brandFooterLabel;
};

} // namespace lumen::ui
