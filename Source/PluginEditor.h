#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "ui/FileSlotComponent.h"
#include "ui/KnobComponent.h"
#include "ui/LedMeterComponent.h"
#include "ui/LumenLookAndFeel.h"
#include "ui/PresetBarComponent.h"
#include "ui/SignalFlowStrip.h"

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
    void refreshPresetList();
    void refreshFileSlotLabels();
    void refreshMeters();
    void refreshSignalFlow();
    void showAudioSettings();

    LumenDSPAudioProcessor& audioProcessor;
    lumen::ui::LumenLookAndFeel lookAndFeel;

    juce::Label titleLabel;
    juce::Label subtitleLabel;
    juce::TextButton audioSettingsButton{"Audio"};

    lumen::ui::SignalFlowStrip signalFlowStrip;
    lumen::ui::PresetBarComponent presetBar;
    lumen::ui::FileSlotComponent namSlot{"NAM Model", "*.nam"};
    lumen::ui::FileSlotComponent irSlot{"Cab IR", "*.wav"};

    lumen::ui::LedMeterComponent inputMeter{"IN"};
    lumen::ui::LedMeterComponent outputMeter{"OUT"};

    lumen::ui::KnobComponent inputGainKnob{"Input", "dB"};
    lumen::ui::KnobComponent gateKnob{"Gate", "dB"};
    lumen::ui::KnobComponent bassKnob{"Bass", "dB"};
    lumen::ui::KnobComponent midKnob{"Mid", "dB"};
    lumen::ui::KnobComponent trebleKnob{"Treble", "dB"};
    lumen::ui::KnobComponent outputKnob{"Output", "dB"};

    juce::ToggleButton gateEnableButton{"Gate"};
    juce::ToggleButton eqEnableButton{"EQ"};
    juce::ToggleButton cabEnableButton{"Cab"};

    std::unique_ptr<SliderAttachment> inputGainAttachment;
    std::unique_ptr<SliderAttachment> gateAttachment;
    std::unique_ptr<SliderAttachment> bassAttachment;
    std::unique_ptr<SliderAttachment> midAttachment;
    std::unique_ptr<SliderAttachment> trebleAttachment;
    std::unique_ptr<SliderAttachment> outputAttachment;
    std::unique_ptr<ButtonAttachment> gateEnableAttachment;
    std::unique_ptr<ButtonAttachment> eqEnableAttachment;
    std::unique_ptr<ButtonAttachment> cabEnableAttachment;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(LumenDSPAudioProcessorEditor)
};
