#include "PluginEditor.h"
#include "parameters/ParameterIds.h"

#if JucePlugin_Build_Standalone
 #include <juce_audio_plugin_client/Standalone/juce_StandaloneFilterWindow.h>
#endif

bool LumenWebBrowser::pageAboutToLoad(const juce::String& newURL)
{
    // macOS/Linux/iOS use juce://juce.backend/; Windows/Android use https://juce.backend/
    if (newURL == getResourceProviderRoot()
        || newURL.startsWith("juce://juce.backend")
        || newURL.startsWith("http://juce.backend")
        || newURL.startsWith("https://juce.backend")
        || newURL.startsWith("file:")
        || newURL.startsWith("data:"))
        return true;

    return false;
}

LumenDSPAudioProcessorEditor::LumenDSPAudioProcessorEditor(LumenDSPAudioProcessor& audioProcessorToEdit)
    : AudioProcessorEditor(&audioProcessorToEdit)
    , audioProcessor(audioProcessorToEdit)
    , webResourceRoot(getWebResourceRoot())
    , webView([this]() {
        auto options = juce::WebBrowserComponent::Options{}
                           .withNativeIntegrationEnabled()
                           .withKeepPageLoadedWhenBrowserIsHidden()
                           .withEventListener("lumenMessage", [this](const juce::var& payload) {
                               const auto message = payload.isString()
                                                        ? payload.toString()
                                                        : juce::JSON::toString(payload);
                               if (message.isNotEmpty())
                                   handleWebMessage(message);
                           })
                           .withNativeFunction(
                               "lumenPostMessage",
                               [this](const juce::Array<juce::var>& args, auto complete) {
                                   if (!args.isEmpty())
                                   {
                                       const auto& arg = args.getReference(0);
                                       const auto message = arg.isString()
                                                                ? arg.toString()
                                                                : juce::JSON::toString(arg);
                                       handleWebMessage(message);
                                   }
                                   complete(juce::var());
                               })
                           .withUserScript(R"JS(
window.lumenPostMessage = function(payload) {
  try {
    const data = (typeof payload === 'string') ? payload : JSON.stringify(payload);
    if (window.__JUCE__ && window.__JUCE__.backend && window.__JUCE__.backend.emitEvent)
      window.__JUCE__.backend.emitEvent('lumenMessage', data);
  } catch (e) {}
};
)JS");

#if JUCE_WINDOWS
        options = options.withBackend(juce::WebBrowserComponent::Options::Backend::webview2)
                      .withWinWebView2Options(
                          juce::WebBrowserComponent::Options::WinWebView2{}.withUserDataFolder(
                              juce::File::getSpecialLocation(juce::File::tempDirectory)
                                  .getChildFile("LumenDSPWebView2")));
#endif

#if JUCE_WEB_BROWSER_RESOURCE_PROVIDER_AVAILABLE
        // Resource provider root is platform-specific (juce:// on Mac/Linux, https:// on Win).
        // No allowedOrigin needed when serving only from the embedded resource provider.
        options = options.withResourceProvider(
            [this](const auto& url) { return getResource(url); });
#endif

        return options;
    }())
{
    setSize(1120, 760);
    setResizeLimits(960, 640, 1800, 1200);
    setResizable(true, true);

    addAndMakeVisible(webView);

    // Guitar amp sim needs live input; JUCE standalone defaults to mute-input (anti-feedback).
    ensureStandaloneInputUnmuted();

#if JUCE_WEB_BROWSER_RESOURCE_PROVIDER_AVAILABLE
    webView.goToURL(juce::WebBrowserComponent::getResourceProviderRoot());
#else
    const auto indexFile = webResourceRoot.getChildFile("index.html");
    if (indexFile.existsAsFile())
        webView.goToURL(juce::URL(indexFile).toString(false));
#endif

    startTimerHz(30);
}

LumenDSPAudioProcessorEditor::~LumenDSPAudioProcessorEditor()
{
    stopTimer();
}

void LumenDSPAudioProcessorEditor::paint(juce::Graphics& graphics)
{
    graphics.fillAll(juce::Colour::fromRGB(17, 17, 22));
}

void LumenDSPAudioProcessorEditor::resized()
{
    webView.setBounds(getLocalBounds());
}

void LumenDSPAudioProcessorEditor::timerCallback()
{
    // Keep re-asserting unmute in case the host settings window reloads mute-on-default.
    ensureStandaloneInputUnmuted();

    if (!webReady)
        return;

    const auto& namEngine = audioProcessor.getAudioPipeline().getNamEngine();
    const bool loading = namEngine.isModelLoading();
    const bool loaded = namEngine.isModelLoaded();
    const auto modelName = namEngine.getLoadedModelName();
    const auto status = [&]() -> juce::String {
        if (loading)
            return "Loading NAM model...";
        if (loaded)
            return "Model: " + modelName;
        if (namEngine.getLastErrorMessage().isNotEmpty())
            return namEngine.getLastErrorMessage();
        return "Ready — load a NAM model or factory preset";
    }();

    // Push full state when model load status changes (async load used to leave UI stuck on "Loading…").
    if (loading != lastModelLoading
        || loaded != lastModelLoaded
        || modelName != lastModelName
        || status != lastStatus)
    {
        lastModelLoading = loading;
        lastModelLoaded = loaded;
        lastModelName = modelName;
        lastStatus = status;
        pushStateToWeb();
    }

    pushMetersToWeb();
    pushPracticeToWeb();
}

void LumenDSPAudioProcessorEditor::ensureStandaloneInputUnmuted()
{
#if JucePlugin_Build_Standalone
    if (auto* holder = juce::StandalonePluginHolder::getInstance())
    {
        if (static_cast<bool>(holder->getMuteInputValue().getValue()))
            holder->getMuteInputValue() = false;

        if (auto* settings = holder->settings.get())
            if (settings->getBoolValue("shouldMuteInput", false))
                settings->setValue("shouldMuteInput", false);
    }
#endif
}

void LumenDSPAudioProcessorEditor::handleWebMessage(const juce::String& message)
{
    const auto parsed = juce::JSON::parse(message);
    if (!parsed.isObject())
        return;

    const auto type = parsed.getProperty("type", {}).toString();

    if (type == "ready")
    {
        webReady = true;
        pushStateToWeb();
        return;
    }

    if (type == "setParameter")
    {
        const auto parameterId = parsed.getProperty("id", {}).toString();
        const float value = static_cast<float>(static_cast<double>(parsed.getProperty("value", 0.0)));
        handleSetParameter(parameterId, value);
        return;
    }

    if (type == "loadPreset")
    {
        audioProcessor.applyPreset(parsed.getProperty("name", {}).toString());
        pushStateToWeb();
        return;
    }

    if (type == "savePreset")
    {
        audioProcessor.storePreset(parsed.getProperty("name", {}).toString());
        pushStateToWeb();
        return;
    }

    if (type == "browseNam")
    {
        handleBrowseNam();
        return;
    }

    if (type == "browseIr")
    {
        handleBrowseIr();
        return;
    }

    if (type == "openAudioSettings")
        handleOpenAudioSettings();
}

void LumenDSPAudioProcessorEditor::pushStateToWeb()
{
    emitToWeb(buildStateObject());
}

void LumenDSPAudioProcessorEditor::pushMetersToWeb()
{
    auto* object = new juce::DynamicObject();
    object->setProperty("type", "meters");
    object->setProperty("inputDb", audioProcessor.getAudioPipeline().getInputMeter().getPeakLevelDb());
    object->setProperty("inputHoldDb", audioProcessor.getAudioPipeline().getInputMeter().getPeakHoldLevelDb());
    object->setProperty("outputDb", audioProcessor.getAudioPipeline().getOutputMeter().getPeakLevelDb());
    object->setProperty("outputHoldDb", audioProcessor.getAudioPipeline().getOutputMeter().getPeakHoldLevelDb());
    emitToWeb(juce::var(object));
}

void LumenDSPAudioProcessorEditor::pushPracticeToWeb()
{
    const auto& pipeline = audioProcessor.getAudioPipeline();
    const auto& tuner = pipeline.getTuner();
    const auto& metro = pipeline.getMetronome();
    auto* object = new juce::DynamicObject();
    object->setProperty("type", "practice");
    object->setProperty("note", tuner.getNoteName());
    object->setProperty("frequencyHz", tuner.getFrequencyHz());
    object->setProperty("cents", tuner.getCentsOffset());
    object->setProperty("locked", tuner.isLocked());
    object->setProperty(
        "metronomeEnabled",
        readParameter(lumen::parameters::metronomeEnabledId) > 0.5f);
    object->setProperty("metronomeBpm", readParameter(lumen::parameters::metronomeBpmId));
    object->setProperty("metronomeVolume", readParameter(lumen::parameters::metronomeVolumeId));
    object->setProperty("beatInBar", metro.getBeatInBar());
    object->setProperty("beatsPerBar", metro.getBeatsPerBar());
    emitToWeb(juce::var(object));
}

void LumenDSPAudioProcessorEditor::emitToWeb(const juce::var& payload)
{
    const auto json = juce::JSON::toString(payload, false);
    const auto script = "window.__lumenReceive && window.__lumenReceive(" + json + ");";
    webView.evaluateJavascript(script);
}

juce::var LumenDSPAudioProcessorEditor::buildStateObject() const
{
    auto* object = new juce::DynamicObject();
    object->setProperty("type", "state");

    auto* parameters = new juce::DynamicObject();
    auto putFloat = [&](const char* key, const char* id) {
        parameters->setProperty(key, readParameter(id));
    };
    auto putBool = [&](const char* key, const char* id) {
        parameters->setProperty(key, readParameter(id) > 0.5f);
    };

    putFloat("inputGain", lumen::parameters::inputGainId);
    putFloat("noiseGateThreshold", lumen::parameters::noiseGateThresholdId);
    putBool("noiseGateEnabled", lumen::parameters::noiseGateEnabledId);
    putFloat("outputLevel", lumen::parameters::outputLevelId);
    putFloat("bassGain", lumen::parameters::bassGainId);
    putFloat("midGain", lumen::parameters::midGainId);
    putFloat("trebleGain", lumen::parameters::trebleGainId);
    putBool("eqEnabled", lumen::parameters::eqEnabledId);
    putBool("cabEnabled", lumen::parameters::cabEnabledId);
    putBool("metronomeEnabled", lumen::parameters::metronomeEnabledId);
    putFloat("metronomeBpm", lumen::parameters::metronomeBpmId);
    putFloat("metronomeVolume", lumen::parameters::metronomeVolumeId);

    putBool("compressorEnabled", lumen::parameters::compressorEnabledId);
    putFloat("compressorThreshold", lumen::parameters::compressorThresholdId);
    putFloat("compressorRatio", lumen::parameters::compressorRatioId);
    putFloat("compressorMix", lumen::parameters::compressorMixId);

    putBool("driveEnabled", lumen::parameters::driveEnabledId);
    putFloat("driveAmount", lumen::parameters::driveAmountId);
    putFloat("driveTone", lumen::parameters::driveToneId);
    putFloat("driveLevel", lumen::parameters::driveLevelId);

    putBool("delayEnabled", lumen::parameters::delayEnabledId);
    putFloat("delayTime", lumen::parameters::delayTimeId);
    putFloat("delayFeedback", lumen::parameters::delayFeedbackId);
    putFloat("delayMix", lumen::parameters::delayMixId);

    putBool("reverbEnabled", lumen::parameters::reverbEnabledId);
    putFloat("reverbSize", lumen::parameters::reverbSizeId);
    putFloat("reverbDamping", lumen::parameters::reverbDampingId);
    putFloat("reverbMix", lumen::parameters::reverbMixId);

    object->setProperty("parameters", juce::var(parameters));

    juce::Array<juce::var> presets;
    for (const auto& name : audioProcessor.getPresetManager().getPresetNames())
        presets.add(name);
    object->setProperty("presets", presets);
    object->setProperty("currentPreset", audioProcessor.getPresetManager().getCurrentPresetName());

    const auto& namEngine = audioProcessor.getAudioPipeline().getNamEngine();
    const auto& ir = audioProcessor.getAudioPipeline().getIrConvolver();
    object->setProperty("namLoaded", namEngine.isModelLoaded());
    object->setProperty("namName", namEngine.getLoadedModelName());
    object->setProperty("irLoaded", ir.isLoaded());
    object->setProperty("irName", ir.getLoadedFileName());

    juce::String status = "Ready — load a NAM model or factory preset";
    if (namEngine.isModelLoading())
        status = "Loading NAM model...";
    else if (namEngine.isModelLoaded())
        status = "Model: " + namEngine.getLoadedModelName();
    else if (namEngine.getLastErrorMessage().isNotEmpty())
        status = namEngine.getLastErrorMessage();
    object->setProperty("status", status);

    return juce::var(object);
}

juce::File LumenDSPAudioProcessorEditor::getWebResourceRoot() const
{
    const auto executable = juce::File::getSpecialLocation(juce::File::currentExecutableFile);

#if JUCE_MAC
    const auto bundleWeb = executable.getParentDirectory()
                               .getParentDirectory()
                               .getChildFile("Resources")
                               .getChildFile("web");
    if (bundleWeb.isDirectory())
        return bundleWeb;
#endif

    const auto beside = executable.getParentDirectory().getChildFile("web");
    if (beside.isDirectory())
        return beside;

    const auto cwd = juce::File::getCurrentWorkingDirectory().getChildFile("Resources").getChildFile("web");
    if (cwd.isDirectory())
        return cwd;

    return juce::File::getCurrentWorkingDirectory()
        .getChildFile("..")
        .getChildFile("Resources")
        .getChildFile("web");
}

std::optional<juce::WebBrowserComponent::Resource> LumenDSPAudioProcessorEditor::getResource(const juce::String& url)
{
    auto path = url.upToFirstOccurrenceOf("?", false, false);
    path = juce::URL::removeEscapeChars(path);

    if (path.isEmpty() || path == "/")
        path = "index.html";
    else if (path.startsWithChar('/'))
        path = path.fromFirstOccurrenceOf("/", false, false);

    if (path.isEmpty())
        path = "index.html";

    auto file = webResourceRoot.getChildFile(path);
    if (!file.existsAsFile() && path != "index.html")
        return std::nullopt;

    if (!file.existsAsFile())
        file = webResourceRoot.getChildFile("index.html");

    if (!file.existsAsFile())
        return std::nullopt;

    juce::MemoryBlock block;
    if (!file.loadFileAsData(block))
        return std::nullopt;

    juce::WebBrowserComponent::Resource resource;
    resource.data.resize(static_cast<size_t>(block.getSize()));
    if (block.getSize() > 0)
        std::memcpy(resource.data.data(), block.getData(), static_cast<size_t>(block.getSize()));
    resource.mimeType = getMimeForExtension(file.getFileExtension().toLowerCase());
    return resource;
}

juce::String LumenDSPAudioProcessorEditor::getMimeForExtension(const juce::String& extension) const
{
    if (extension == ".html" || extension == ".htm")
        return "text/html";
    if (extension == ".css")
        return "text/css";
    if (extension == ".js")
        return "application/javascript";
    if (extension == ".svg")
        return "image/svg+xml";
    if (extension == ".png")
        return "image/png";
    if (extension == ".jpg" || extension == ".jpeg")
        return "image/jpeg";
    if (extension == ".woff2")
        return "font/woff2";
    return "application/octet-stream";
}

void LumenDSPAudioProcessorEditor::handleSetParameter(const juce::String& parameterId, float value)
{
    auto* parameter = audioProcessor.getValueTreeState().getParameter(parameterId);
    if (parameter == nullptr)
        return;

    if (auto* ranged = dynamic_cast<juce::RangedAudioParameter*>(parameter))
    {
        ranged->setValueNotifyingHost(ranged->convertTo0to1(value));
        return;
    }

    parameter->setValueNotifyingHost(value);
}

void LumenDSPAudioProcessorEditor::handleBrowseNam()
{
    fileChooser = std::make_unique<juce::FileChooser>("Select NAM model", juce::File{}, "*.nam");
    constexpr auto flags = juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles;
    fileChooser->launchAsync(flags, [this](const juce::FileChooser& chooser) {
        const auto result = chooser.getResult();
        if (result.existsAsFile())
        {
            audioProcessor.requestNamLoad(result);
            juce::Timer::callAfterDelay(400, [this]() { pushStateToWeb(); });
            juce::Timer::callAfterDelay(1200, [this]() { pushStateToWeb(); });
        }
    });
}

void LumenDSPAudioProcessorEditor::handleBrowseIr()
{
    fileChooser = std::make_unique<juce::FileChooser>("Select cabinet IR", juce::File{}, "*.wav");
    constexpr auto flags = juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles;
    fileChooser->launchAsync(flags, [this](const juce::FileChooser& chooser) {
        const auto result = chooser.getResult();
        if (result.existsAsFile())
        {
            audioProcessor.requestIrLoad(result);
            juce::Timer::callAfterDelay(400, [this]() { pushStateToWeb(); });
            juce::Timer::callAfterDelay(1200, [this]() { pushStateToWeb(); });
        }
    });
}

void LumenDSPAudioProcessorEditor::handleOpenAudioSettings()
{
#if JucePlugin_Build_Standalone
    if (auto* holder = juce::StandalonePluginHolder::getInstance())
    {
        holder->showAudioSettingsDialog();
        return;
    }
#endif
}

float LumenDSPAudioProcessorEditor::readParameter(const juce::String& parameterId) const
{
    auto* parameter = audioProcessor.getValueTreeState().getParameter(parameterId);
    if (parameter == nullptr)
        return 0.0f;

    if (auto* ranged = dynamic_cast<juce::RangedAudioParameter*>(parameter))
        return ranged->convertFrom0to1(ranged->getValue());

    return parameter->getValue();
}
