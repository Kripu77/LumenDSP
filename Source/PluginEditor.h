#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "ui/AmpFaceplateComponent.h"
#include "ui/CabStageComponent.h"
#include "ui/EqRackComponent.h"
#include "ui/InputStageComponent.h"
#include "ui/LumenLookAndFeel.h"
#include "ui/SectionNavComponent.h"
#include "ui/TopChromeComponent.h"

class LumenDSPAudioProcessorEditor : public juce::AudioProcessorEditor,
                                     private juce::Timer
{
public:
    explicit LumenDSPAudioProcessorEditor(LumenDSPAudioProcessor& audioProcessorToEdit);
    ~LumenDSPAudioProcessorEditor() override;

    void paint(juce::Graphics& graphics) override;
    void resized() override;

private:
    using SliderAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;
    using ButtonAttachment = juce::AudioProcessorValueTreeState::ButtonAttachment;

    void timerCallback() override;
    void setupControlAttachments();
    void setupFileSlots();
    void setupPresetBar();
    void setupSectionNavigation();
    void showSection(lumen::ui::EditorSection section);
    void refreshPresetList();
    void refreshFileSlotLabels();
    void refreshMeters();
    void refreshHardwareState();
    void showAudioSettings();

    LumenDSPAudioProcessor& audioProcessor;
    lumen::ui::LumenLookAndFeel lookAndFeel;

    lumen::ui::TopChromeComponent topChrome;
    lumen::ui::SectionNavComponent sectionNav;
    lumen::ui::InputStageComponent inputStage;
    lumen::ui::AmpFaceplateComponent ampStage;
    lumen::ui::EqRackComponent eqStage;
    lumen::ui::CabStageComponent cabStage;

    std::unique_ptr<SliderAttachment> inputGainAttachment;
    std::unique_ptr<SliderAttachment> gateAttachment;
    std::unique_ptr<SliderAttachment> outputAttachment;
    std::unique_ptr<SliderAttachment> bassAttachment;
    std::unique_ptr<SliderAttachment> midAttachment;
    std::unique_ptr<SliderAttachment> trebleAttachment;
    std::unique_ptr<ButtonAttachment> gateEnableAttachment;
    std::unique_ptr<ButtonAttachment> eqEnableAttachment;
    std::unique_ptr<ButtonAttachment> cabEnableAttachment;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(LumenDSPAudioProcessorEditor)
};
