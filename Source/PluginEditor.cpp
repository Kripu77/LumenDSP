#include "PluginEditor.h"

#if JucePlugin_Build_Standalone
 #include <juce_audio_plugin_client/Standalone/juce_StandaloneFilterWindow.h>
#endif

namespace
{

constexpr int headerHeightPixels = 64;
constexpr int signalFlowHeightPixels = 64;
constexpr int presetBarHeightPixels = 40;
constexpr int fileSlotHeightPixels = 88;
constexpr int toggleRowHeightPixels = 28;
constexpr int footerHeightPixels = 8;

} // namespace

LumenDSPAudioProcessorEditor::LumenDSPAudioProcessorEditor(LumenDSPAudioProcessor& audioProcessorToEdit)
    : AudioProcessorEditor(&audioProcessorToEdit)
    , audioProcessor(audioProcessorToEdit)
{
    setLookAndFeel(&lookAndFeel);
    setSize(lumen::design::windowWidthPixels, lumen::design::windowHeightPixels);
    setResizeLimits(
        lumen::design::windowMinimumWidthPixels,
        lumen::design::windowMinimumHeightPixels,
        1400,
        900);
    setResizable(true, true);

    titleLabel.setText("LumenDSP", juce::dontSendNotification);
    titleLabel.setFont(lumen::design::titleFont());
    titleLabel.setColour(juce::Label::textColourId, lumen::design::textPrimary());
    addAndMakeVisible(titleLabel);

    const auto factoryStatus = audioProcessor.getFactoryStatusMessage();
    const auto subtitleText = factoryStatus.isNotEmpty()
                                  ? factoryStatus
                                  : juce::String("Modern fusion NAM player");
    subtitleLabel.setText(subtitleText, juce::dontSendNotification);
    subtitleLabel.setFont(lumen::design::bodyFont());
    subtitleLabel.setColour(juce::Label::textColourId, lumen::design::textMuted());
    addAndMakeVisible(subtitleLabel);

    audioSettingsButton.onClick = [this]() { showAudioSettings(); };
    addAndMakeVisible(audioSettingsButton);

    addAndMakeVisible(signalFlowStrip);
    addAndMakeVisible(presetBar);
    addAndMakeVisible(namSlot);
    addAndMakeVisible(irSlot);
    addAndMakeVisible(inputMeter);
    addAndMakeVisible(outputMeter);

    addAndMakeVisible(inputGainKnob);
    addAndMakeVisible(gateKnob);
    addAndMakeVisible(bassKnob);
    addAndMakeVisible(midKnob);
    addAndMakeVisible(trebleKnob);
    addAndMakeVisible(outputKnob);

    addAndMakeVisible(gateEnableButton);
    addAndMakeVisible(eqEnableButton);
    addAndMakeVisible(cabEnableButton);

    setupControlAttachments();
    setupFileSlots();
    setupPresetBar();
    refreshPresetList();
    refreshFileSlotLabels();
    refreshSignalFlow();

    startTimerHz(static_cast<int>(lumen::design::uiTimerIntervalHertz));
}

LumenDSPAudioProcessorEditor::~LumenDSPAudioProcessorEditor()
{
    stopTimer();
    setLookAndFeel(nullptr);
}

void LumenDSPAudioProcessorEditor::paint(juce::Graphics& graphics)
{
    graphics.fillAll(lumen::design::backgroundDeep());

    auto content = getLocalBounds().reduced(lumen::design::spacingTwoUnitsPixels);
    auto knobsPanel = content;
    knobsPanel.removeFromTop(headerHeightPixels + lumen::design::spacingUnitPixels);
    knobsPanel.removeFromTop(signalFlowHeightPixels + lumen::design::spacingUnitPixels);
    knobsPanel.removeFromTop(presetBarHeightPixels + lumen::design::spacingUnitPixels);
    knobsPanel.removeFromTop(fileSlotHeightPixels + lumen::design::spacingUnitPixels);
    knobsPanel.removeFromBottom(toggleRowHeightPixels + lumen::design::spacingUnitPixels + footerHeightPixels);
    knobsPanel.reduce(lumen::design::meterWidthPixels + lumen::design::spacingTwoUnitsPixels, 0);

    graphics.setColour(lumen::design::backgroundPanel().withAlpha(lumen::design::panelBackgroundAlpha));
    graphics.fillRoundedRectangle(
        knobsPanel.toFloat(),
        static_cast<float>(lumen::design::panelCornerRadiusPixels));
    graphics.setColour(lumen::design::borderSubtle());
    graphics.drawRoundedRectangle(
        knobsPanel.toFloat(),
        static_cast<float>(lumen::design::panelCornerRadiusPixels),
        1.0f);
}

void LumenDSPAudioProcessorEditor::resized()
{
    auto bounds = getLocalBounds().reduced(lumen::design::spacingTwoUnitsPixels);

    auto header = bounds.removeFromTop(headerHeightPixels);
    audioSettingsButton.setBounds(header.removeFromRight(88).withSizeKeepingCentre(88, 32));
    auto titleArea = header.removeFromLeft(header.getWidth() / 2);
    titleLabel.setBounds(titleArea.removeFromTop(32));
    subtitleLabel.setBounds(titleArea);

    bounds.removeFromTop(lumen::design::spacingUnitPixels);
    signalFlowStrip.setBounds(bounds.removeFromTop(signalFlowHeightPixels));

    bounds.removeFromTop(lumen::design::spacingUnitPixels);
    presetBar.setBounds(bounds.removeFromTop(presetBarHeightPixels));

    bounds.removeFromTop(lumen::design::spacingUnitPixels);
    auto fileRow = bounds.removeFromTop(fileSlotHeightPixels);
    namSlot.setBounds(fileRow.removeFromLeft(fileRow.getWidth() / 2).reduced(0, 0).withTrimmedRight(6));
    irSlot.setBounds(fileRow.withTrimmedLeft(6));

    bounds.removeFromTop(lumen::design::spacingUnitPixels);
    auto toggleRow = bounds.removeFromBottom(toggleRowHeightPixels);
    bounds.removeFromBottom(lumen::design::spacingUnitPixels);

    const int toggleWidth = 100;
    gateEnableButton.setBounds(toggleRow.removeFromLeft(toggleWidth));
    toggleRow.removeFromLeft(lumen::design::spacingUnitPixels);
    eqEnableButton.setBounds(toggleRow.removeFromLeft(toggleWidth));
    toggleRow.removeFromLeft(lumen::design::spacingUnitPixels);
    cabEnableButton.setBounds(toggleRow.removeFromLeft(toggleWidth));

    auto meterLeft = bounds.removeFromLeft(lumen::design::meterWidthPixels + lumen::design::spacingUnitPixels);
    auto meterRight = bounds.removeFromRight(lumen::design::meterWidthPixels + lumen::design::spacingUnitPixels);
    inputMeter.setBounds(meterLeft.reduced(2));
    outputMeter.setBounds(meterRight.reduced(2));

    bounds.reduce(lumen::design::spacingTwoUnitsPixels, lumen::design::spacingTwoUnitsPixels);

    const int knobCount = 6;
    const int knobWidth = bounds.getWidth() / knobCount;
    inputGainKnob.setBounds(bounds.removeFromLeft(knobWidth).reduced(4));
    gateKnob.setBounds(bounds.removeFromLeft(knobWidth).reduced(4));
    bassKnob.setBounds(bounds.removeFromLeft(knobWidth).reduced(4));
    midKnob.setBounds(bounds.removeFromLeft(knobWidth).reduced(4));
    trebleKnob.setBounds(bounds.removeFromLeft(knobWidth).reduced(4));
    outputKnob.setBounds(bounds.reduced(4));
}

void LumenDSPAudioProcessorEditor::timerCallback()
{
    refreshMeters();
    refreshFileSlotLabels();
    refreshSignalFlow();
}

void LumenDSPAudioProcessorEditor::setupControlAttachments()
{
    auto& state = audioProcessor.getValueTreeState();

    inputGainAttachment = std::make_unique<SliderAttachment>(
        state,
        lumen::parameters::inputGainId,
        inputGainKnob.getSlider());
    gateAttachment = std::make_unique<SliderAttachment>(
        state,
        lumen::parameters::noiseGateThresholdId,
        gateKnob.getSlider());
    bassAttachment = std::make_unique<SliderAttachment>(
        state,
        lumen::parameters::bassGainId,
        bassKnob.getSlider());
    midAttachment = std::make_unique<SliderAttachment>(
        state,
        lumen::parameters::midGainId,
        midKnob.getSlider());
    trebleAttachment = std::make_unique<SliderAttachment>(
        state,
        lumen::parameters::trebleGainId,
        trebleKnob.getSlider());
    outputAttachment = std::make_unique<SliderAttachment>(
        state,
        lumen::parameters::outputLevelId,
        outputKnob.getSlider());

    gateEnableAttachment = std::make_unique<ButtonAttachment>(
        state,
        lumen::parameters::noiseGateEnabledId,
        gateEnableButton);
    eqEnableAttachment = std::make_unique<ButtonAttachment>(
        state,
        lumen::parameters::eqEnabledId,
        eqEnableButton);
    cabEnableAttachment = std::make_unique<ButtonAttachment>(
        state,
        lumen::parameters::cabEnabledId,
        cabEnableButton);
}

void LumenDSPAudioProcessorEditor::setupFileSlots()
{
    namSlot.onFileChosen = [this](const juce::File& file) {
        namSlot.setBusy(true);
        namSlot.setStatusText("Loading model...");
        audioProcessor.requestNamLoad(file);
    };

    irSlot.onFileChosen = [this](const juce::File& file) {
        irSlot.setBusy(true);
        irSlot.setStatusText("Loading IR...");
        audioProcessor.requestIrLoad(file);
    };
}

void LumenDSPAudioProcessorEditor::setupPresetBar()
{
    presetBar.onSaveRequested = [this]() {
        const auto name = presetBar.getEditorPresetName();

        if (name.isEmpty())
            return;

        if (audioProcessor.storePreset(name))
            refreshPresetList();
    };

    presetBar.onLoadRequested = [this]() {
        const auto name = presetBar.getSelectedPresetName();

        if (name.isEmpty())
            return;

        audioProcessor.applyPreset(name);
        refreshFileSlotLabels();
    };

    presetBar.onDeleteRequested = [this]() {
        const auto name = presetBar.getSelectedPresetName();

        if (name.isEmpty())
            return;

        audioProcessor.getPresetManager().deletePreset(name);
        refreshPresetList();
    };

    presetBar.onPresetSelected = [this](const juce::String& name) {
        if (name.isEmpty())
            return;

        audioProcessor.applyPreset(name);
        refreshFileSlotLabels();
    };
}

void LumenDSPAudioProcessorEditor::refreshPresetList()
{
    auto& presetManager = audioProcessor.getPresetManager();
    presetBar.setPresetNames(presetManager.getPresetNames(), presetManager.getCurrentPresetName());
}

void LumenDSPAudioProcessorEditor::refreshFileSlotLabels()
{
    auto& pipeline = audioProcessor.getAudioPipeline();
    auto& namEngine = pipeline.getNamEngine();
    auto& irConvolver = pipeline.getIrConvolver();

    if (namEngine.isModelLoading())
    {
        namSlot.setBusy(true);
        namSlot.setStatusText("Loading model...");
    }
    else if (namEngine.isModelLoaded())
    {
        namSlot.setBusy(false);
        namSlot.setFileName(namEngine.getLoadedModelName());
        namSlot.setStatusText("Ready");
    }
    else
    {
        namSlot.setBusy(false);
        const auto error = namEngine.getLastErrorMessage();
        namSlot.setStatusText(error.isNotEmpty() ? error : "Drop a .nam capture");
    }

    if (irConvolver.isLoaded())
    {
        irSlot.setBusy(false);
        irSlot.setFileName(irConvolver.getLoadedFileName());
        irSlot.setStatusText("Ready");
    }
    else
    {
        irSlot.setBusy(false);
        irSlot.setStatusText("Drop a .wav IR");
    }
}

void LumenDSPAudioProcessorEditor::refreshMeters()
{
    auto& pipeline = audioProcessor.getAudioPipeline();
    inputMeter.setLevels(
        pipeline.getInputMeter().getPeakLevelDb(),
        pipeline.getInputMeter().getPeakHoldLevelDb());
    outputMeter.setLevels(
        pipeline.getOutputMeter().getPeakLevelDb(),
        pipeline.getOutputMeter().getPeakHoldLevelDb());
}

void LumenDSPAudioProcessorEditor::refreshSignalFlow()
{
    auto& state = audioProcessor.getValueTreeState();
    auto& pipeline = audioProcessor.getAudioPipeline();

    const bool gateOn = state.getRawParameterValue(lumen::parameters::noiseGateEnabledId)->load() > 0.5f;
    const bool eqOn = state.getRawParameterValue(lumen::parameters::eqEnabledId)->load() > 0.5f;
    const bool cabOn = state.getRawParameterValue(lumen::parameters::cabEnabledId)->load() > 0.5f;
    const bool ampLoaded = pipeline.getNamEngine().isModelLoaded();
    const bool irLoaded = pipeline.getIrConvolver().isLoaded();

    signalFlowStrip.setStageActive(lumen::ui::SignalFlowStrip::Stage::input, true);
    signalFlowStrip.setStageActive(lumen::ui::SignalFlowStrip::Stage::amp, ampLoaded);
    signalFlowStrip.setStageActive(lumen::ui::SignalFlowStrip::Stage::eq, eqOn);
    signalFlowStrip.setStageActive(lumen::ui::SignalFlowStrip::Stage::cab, cabOn && irLoaded);
    signalFlowStrip.setStageActive(lumen::ui::SignalFlowStrip::Stage::output, true);

    signalFlowStrip.setStageLabel(
        lumen::ui::SignalFlowStrip::Stage::input,
        gateOn ? "Input + Gate" : "Input");
    signalFlowStrip.setStageLabel(
        lumen::ui::SignalFlowStrip::Stage::amp,
        ampLoaded ? "Amp" : "Amp (empty)");
    signalFlowStrip.setStageLabel(
        lumen::ui::SignalFlowStrip::Stage::cab,
        (cabOn && irLoaded) ? "Cab" : "Cab (bypass)");
}

void LumenDSPAudioProcessorEditor::showAudioSettings()
{
#if JucePlugin_Build_Standalone
    if (auto* holder = juce::StandalonePluginHolder::getInstance())
    {
        holder->showAudioSettingsDialog();
        return;
    }
#endif

    juce::AlertWindow::showMessageBoxAsync(
        juce::AlertWindow::InfoIcon,
        "Audio Settings",
        "Device selection is available in the standalone build via this control. "
        "When running as a VST3, choose input and output devices in your DAW.");
}
