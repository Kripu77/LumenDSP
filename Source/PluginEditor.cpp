#include "PluginEditor.h"

#if JucePlugin_Build_Standalone
 #include <juce_audio_plugin_client/Standalone/juce_StandaloneFilterWindow.h>
#endif

LumenDSPAudioProcessorEditor::LumenDSPAudioProcessorEditor(LumenDSPAudioProcessor& audioProcessorToEdit)
    : AudioProcessorEditor(&audioProcessorToEdit)
    , audioProcessor(audioProcessorToEdit)
{
    setLookAndFeel(&lookAndFeel);
    setSize(lumen::design::windowWidthPixels, lumen::design::windowHeightPixels);
    setResizeLimits(
        lumen::design::windowMinimumWidthPixels,
        lumen::design::windowMinimumHeightPixels,
        1600,
        1000);
    setResizable(true, true);

    addAndMakeVisible(topChrome);
    addAndMakeVisible(sectionNav);
    addChildComponent(inputStage);
    addAndMakeVisible(ampStage);
    addChildComponent(eqStage);
    addChildComponent(cabStage);

    setupControlAttachments();
    setupFileSlots();
    setupPresetBar();
    setupSectionNavigation();
    refreshPresetList();
    refreshFileSlotLabels();
    refreshHardwareState();
    showSection(lumen::ui::EditorSection::amp);

    topChrome.getAudioButton().onClick = [this]() { showAudioSettings(); };

    startTimerHz(static_cast<int>(lumen::design::uiTimerIntervalHertz));
}

LumenDSPAudioProcessorEditor::~LumenDSPAudioProcessorEditor()
{
    stopTimer();
    setLookAndFeel(nullptr);
}

void LumenDSPAudioProcessorEditor::paint(juce::Graphics& graphics)
{
    lumen::design::fillStudioBackdrop(graphics, getLocalBounds().toFloat());
}

void LumenDSPAudioProcessorEditor::resized()
{
    auto bounds = getLocalBounds().reduced(lumen::design::spacingTwoUnitsPixels);

    topChrome.setBounds(bounds.removeFromTop(lumen::design::topChromeHeightPixels));
    bounds.removeFromTop(lumen::design::spacingUnitPixels);

    sectionNav.setBounds(bounds.removeFromBottom(lumen::design::bottomNavHeightPixels));
    bounds.removeFromBottom(lumen::design::spacingUnitPixels);

    inputStage.setBounds(bounds);
    ampStage.setBounds(bounds);
    eqStage.setBounds(bounds);
    cabStage.setBounds(bounds);
}

void LumenDSPAudioProcessorEditor::timerCallback()
{
    refreshMeters();
    refreshFileSlotLabels();
    refreshHardwareState();
}

void LumenDSPAudioProcessorEditor::setupControlAttachments()
{
    auto& state = audioProcessor.getValueTreeState();

    inputGainAttachment = std::make_unique<SliderAttachment>(
        state, lumen::parameters::inputGainId, topChrome.getInputKnob().getSlider());
    gateAttachment = std::make_unique<SliderAttachment>(
        state, lumen::parameters::noiseGateThresholdId, topChrome.getGateKnob().getSlider());
    outputAttachment = std::make_unique<SliderAttachment>(
        state, lumen::parameters::outputLevelId, topChrome.getOutputKnob().getSlider());

    gateEnableAttachment = std::make_unique<ButtonAttachment>(
        state, lumen::parameters::noiseGateEnabledId, topChrome.getGateEnableButton());

    inputStage.getGatePedal().getPrimaryKnob().getSlider().getValueObject().referTo(
        topChrome.getGateKnob().getSlider().getValueObject());
    inputStage.getGatePedal().getEnableButton().getToggleStateValue().referTo(
        topChrome.getGateEnableButton().getToggleStateValue());

    bassAttachment = std::make_unique<SliderAttachment>(
        state, lumen::parameters::bassGainId, ampStage.getBassKnob().getSlider());
    midAttachment = std::make_unique<SliderAttachment>(
        state, lumen::parameters::midGainId, ampStage.getMidKnob().getSlider());
    trebleAttachment = std::make_unique<SliderAttachment>(
        state, lumen::parameters::trebleGainId, ampStage.getTrebleKnob().getSlider());
    eqEnableAttachment = std::make_unique<ButtonAttachment>(
        state, lumen::parameters::eqEnabledId, ampStage.getEqEnableButton());

    eqStage.getBassSlider().getValueObject().referTo(ampStage.getBassKnob().getSlider().getValueObject());
    eqStage.getMidSlider().getValueObject().referTo(ampStage.getMidKnob().getSlider().getValueObject());
    eqStage.getTrebleSlider().getValueObject().referTo(ampStage.getTrebleKnob().getSlider().getValueObject());
    eqStage.getEnableButton().getToggleStateValue().referTo(ampStage.getEqEnableButton().getToggleStateValue());

    cabEnableAttachment = std::make_unique<ButtonAttachment>(
        state, lumen::parameters::cabEnabledId, cabStage.getEnableButton());
}

void LumenDSPAudioProcessorEditor::setupFileSlots()
{
    ampStage.getModelSlot().onFileChosen = [this](const juce::File& file) {
        ampStage.getModelSlot().setBusy(true);
        ampStage.getModelSlot().setStatusText("Loading model…");
        audioProcessor.requestNamLoad(file);
    };

    cabStage.getIrSlot().onFileChosen = [this](const juce::File& file) {
        cabStage.getIrSlot().setBusy(true);
        cabStage.getIrSlot().setStatusText("Loading IR…");
        audioProcessor.requestIrLoad(file);
    };
}

void LumenDSPAudioProcessorEditor::setupPresetBar()
{
    auto& presetBar = topChrome.getPresetBar();

    presetBar.onSaveRequested = [this]() {
        const auto name = topChrome.getPresetBar().getEditorPresetName();
        if (name.isEmpty())
            return;
        if (audioProcessor.storePreset(name))
            refreshPresetList();
    };

    presetBar.onLoadRequested = [this]() {
        const auto name = topChrome.getPresetBar().getSelectedPresetName();
        if (name.isEmpty())
            return;
        audioProcessor.applyPreset(name);
        refreshFileSlotLabels();
    };

    presetBar.onDeleteRequested = [this]() {
        const auto name = topChrome.getPresetBar().getSelectedPresetName();
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

void LumenDSPAudioProcessorEditor::setupSectionNavigation()
{
    sectionNav.onSectionChanged = [this](lumen::ui::EditorSection section) {
        showSection(section);
    };
}

void LumenDSPAudioProcessorEditor::showSection(lumen::ui::EditorSection section)
{
    inputStage.setVisible(section == lumen::ui::EditorSection::input);
    ampStage.setVisible(section == lumen::ui::EditorSection::amp);
    eqStage.setVisible(section == lumen::ui::EditorSection::eq);
    cabStage.setVisible(section == lumen::ui::EditorSection::cab);
    sectionNav.setActiveSection(section);
}

void LumenDSPAudioProcessorEditor::refreshPresetList()
{
    auto& presetManager = audioProcessor.getPresetManager();
    topChrome.getPresetBar().setPresetNames(
        presetManager.getPresetNames(),
        presetManager.getCurrentPresetName());
}

void LumenDSPAudioProcessorEditor::refreshFileSlotLabels()
{
    auto& pipeline = audioProcessor.getAudioPipeline();
    auto& namEngine = pipeline.getNamEngine();
    auto& irConvolver = pipeline.getIrConvolver();

    if (namEngine.isModelLoading())
    {
        ampStage.getModelSlot().setBusy(true);
        ampStage.getModelSlot().setStatusText("Loading model…");
        ampStage.setModelStatus({}, false, true);
    }
    else if (namEngine.isModelLoaded())
    {
        ampStage.getModelSlot().setBusy(false);
        ampStage.getModelSlot().setFileName(namEngine.getLoadedModelName());
        ampStage.getModelSlot().setStatusText("Ready");
        ampStage.setModelStatus(namEngine.getLoadedModelName(), true, false);
    }
    else
    {
        ampStage.getModelSlot().setBusy(false);
        const auto error = namEngine.getLastErrorMessage();
        ampStage.getModelSlot().setStatusText(error.isNotEmpty() ? error : "Drop a .nam capture");
        ampStage.setModelStatus({}, false, false);
    }

    if (irConvolver.isLoaded())
    {
        cabStage.getIrSlot().setBusy(false);
        cabStage.getIrSlot().setFileName(irConvolver.getLoadedFileName());
        cabStage.getIrSlot().setStatusText("Ready");
        cabStage.setIrStatus(irConvolver.getLoadedFileName(), true);
    }
    else
    {
        cabStage.getIrSlot().setBusy(false);
        cabStage.getIrSlot().setStatusText("Drop a .wav IR");
        cabStage.setIrStatus({}, false);
    }
}

void LumenDSPAudioProcessorEditor::refreshMeters()
{
    auto& pipeline = audioProcessor.getAudioPipeline();
    topChrome.getInputMeter().setLevels(
        pipeline.getInputMeter().getPeakLevelDb(),
        pipeline.getInputMeter().getPeakHoldLevelDb());
    topChrome.getOutputMeter().setLevels(
        pipeline.getOutputMeter().getPeakLevelDb(),
        pipeline.getOutputMeter().getPeakHoldLevelDb());
}

void LumenDSPAudioProcessorEditor::refreshHardwareState()
{
    auto& state = audioProcessor.getValueTreeState();
    const bool gateOn = state.getRawParameterValue(lumen::parameters::noiseGateEnabledId)->load() > 0.5f;
    inputStage.getGatePedal().setEngaged(gateOn);
    eqStage.repaint();
    cabStage.repaint();
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
        "Device selection is available in the standalone build. "
        "When running as a VST3, choose input and output devices in your DAW.");
}
