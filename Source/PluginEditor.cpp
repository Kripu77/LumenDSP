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

    auto& gatePedalSlider = inputStage.getGatePedal().getPrimaryKnob().getSlider();
    gatePedalSlider.setRange(
        lumen::parameters::ranges::noiseGateThresholdMinimumDb,
        lumen::parameters::ranges::noiseGateThresholdMaximumDb,
        lumen::parameters::ranges::noiseGateThresholdStepDb);
    gatePedalSlider.setValue(topChrome.getGateKnob().getSlider().getValue(), juce::dontSendNotification);
    gatePedalSlider.onValueChange = [this]() {
        topChrome.getGateKnob().getSlider().setValue(
            inputStage.getGatePedal().getPrimaryKnob().getSlider().getValue(),
            juce::sendNotificationSync);
    };
    topChrome.getGateKnob().getSlider().onValueChange = [this]() {
        inputStage.getGatePedal().getPrimaryKnob().getSlider().setValue(
            topChrome.getGateKnob().getSlider().getValue(),
            juce::dontSendNotification);
    };

    inputStage.getGatePedal().getEnableButton().onClick = [this]() {
        topChrome.getGateEnableButton().setToggleState(
            inputStage.getGatePedal().getEnableButton().getToggleState(),
            juce::sendNotificationSync);
    };
    topChrome.getGateEnableButton().onClick = [this]() {
        inputStage.getGatePedal().getEnableButton().setToggleState(
            topChrome.getGateEnableButton().getToggleState(),
            juce::dontSendNotification);
    };

    bassAttachment = std::make_unique<SliderAttachment>(
        state, lumen::parameters::bassGainId, ampStage.getBassKnob().getSlider());
    midAttachment = std::make_unique<SliderAttachment>(
        state, lumen::parameters::midGainId, ampStage.getMidKnob().getSlider());
    trebleAttachment = std::make_unique<SliderAttachment>(
        state, lumen::parameters::trebleGainId, ampStage.getTrebleKnob().getSlider());
    eqEnableAttachment = std::make_unique<ButtonAttachment>(
        state, lumen::parameters::eqEnabledId, ampStage.getEqEnableButton());

    auto syncEqSlider = [](juce::Slider& destination, juce::Slider& source) {
        destination.setRange(source.getMinimum(), source.getMaximum(), source.getInterval());
        destination.setValue(source.getValue(), juce::dontSendNotification);
    };
    syncEqSlider(eqStage.getBassSlider(), ampStage.getBassKnob().getSlider());
    syncEqSlider(eqStage.getMidSlider(), ampStage.getMidKnob().getSlider());
    syncEqSlider(eqStage.getTrebleSlider(), ampStage.getTrebleKnob().getSlider());

    eqStage.getBassSlider().onValueChange = [this]() {
        ampStage.getBassKnob().getSlider().setValue(eqStage.getBassSlider().getValue(), juce::sendNotificationSync);
    };
    eqStage.getMidSlider().onValueChange = [this]() {
        ampStage.getMidKnob().getSlider().setValue(eqStage.getMidSlider().getValue(), juce::sendNotificationSync);
    };
    eqStage.getTrebleSlider().onValueChange = [this]() {
        ampStage.getTrebleKnob().getSlider().setValue(eqStage.getTrebleSlider().getValue(), juce::sendNotificationSync);
    };

    ampStage.getBassKnob().getSlider().onValueChange = [this]() {
        eqStage.getBassSlider().setValue(ampStage.getBassKnob().getSlider().getValue(), juce::dontSendNotification);
    };
    ampStage.getMidKnob().getSlider().onValueChange = [this]() {
        eqStage.getMidSlider().setValue(ampStage.getMidKnob().getSlider().getValue(), juce::dontSendNotification);
    };
    ampStage.getTrebleKnob().getSlider().onValueChange = [this]() {
        eqStage.getTrebleSlider().setValue(ampStage.getTrebleKnob().getSlider().getValue(), juce::dontSendNotification);
    };

    eqStage.getEnableButton().setToggleState(ampStage.getEqEnableButton().getToggleState(), juce::dontSendNotification);
    eqStage.getEnableButton().onClick = [this]() {
        ampStage.getEqEnableButton().setToggleState(
            eqStage.getEnableButton().getToggleState(),
            juce::sendNotificationSync);
    };
    ampStage.getEqEnableButton().onClick = [this]() {
        eqStage.getEnableButton().setToggleState(
            ampStage.getEqEnableButton().getToggleState(),
            juce::dontSendNotification);
    };

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

    const float gateThreshold = state.getRawParameterValue(lumen::parameters::noiseGateThresholdId)->load();
    if (std::abs(inputStage.getGatePedal().getPrimaryKnob().getSlider().getValue() - gateThreshold) > 0.05)
        inputStage.getGatePedal().getPrimaryKnob().getSlider().setValue(gateThreshold, juce::dontSendNotification);

    if (std::abs(eqStage.getBassSlider().getValue() - ampStage.getBassKnob().getSlider().getValue()) > 0.05)
        eqStage.getBassSlider().setValue(ampStage.getBassKnob().getSlider().getValue(), juce::dontSendNotification);
    if (std::abs(eqStage.getMidSlider().getValue() - ampStage.getMidKnob().getSlider().getValue()) > 0.05)
        eqStage.getMidSlider().setValue(ampStage.getMidKnob().getSlider().getValue(), juce::dontSendNotification);
    if (std::abs(eqStage.getTrebleSlider().getValue() - ampStage.getTrebleKnob().getSlider().getValue()) > 0.05)
        eqStage.getTrebleSlider().setValue(ampStage.getTrebleKnob().getSlider().getValue(), juce::dontSendNotification);

    eqStage.getEnableButton().setToggleState(
        ampStage.getEqEnableButton().getToggleState(),
        juce::dontSendNotification);

    eqStage.repaint();
    cabStage.repaint();
    ampStage.repaint();
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
