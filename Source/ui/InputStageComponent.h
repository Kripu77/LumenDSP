#pragma once

#include <JuceHeader.h>
#include "ui/DesignTokens.h"
#include "ui/PedalModuleComponent.h"

namespace lumen::ui
{

class InputStageComponent : public juce::Component
{
public:
    InputStageComponent();

    void paint(juce::Graphics& graphics) override;
    void resized() override;

    PedalModuleComponent& getGatePedal() noexcept;

private:
    juce::Label headlineLabel;
    juce::Label bodyLabel;
    PedalModuleComponent gatePedal;
};

} // namespace lumen::ui
