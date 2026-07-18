#include "PluginProcessor.h"
#include "PluginEditor.h"

LumenDSPAudioProcessor::LumenDSPAudioProcessor()
    : AudioProcessor(
          BusesProperties()
              .withInput("Input", juce::AudioChannelSet::stereo(), true)
              .withOutput("Output", juce::AudioChannelSet::stereo(), true))
    , apvts(*this, nullptr, "PARAMETERS", lumen::parameters::createParameterLayout())
    , presetManager(apvts)
{
    bootstrapFactoryExperience();
}

LumenDSPAudioProcessor::~LumenDSPAudioProcessor()
{
    backgroundLoadPool.removeAllJobs(true, 4000);
}

void LumenDSPAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    juce::dsp::ProcessSpec processSpec;
    processSpec.sampleRate = sampleRate;
    processSpec.maximumBlockSize = static_cast<juce::uint32>(samplesPerBlock);
    processSpec.numChannels = static_cast<juce::uint32>(
        juce::jmax(getTotalNumInputChannels(), getTotalNumOutputChannels()));

    audioPipeline.prepare(processSpec);
    audioPipeline.setControlState(readControlState());
}

void LumenDSPAudioProcessor::releaseResources()
{
    audioPipeline.reset();
}

bool LumenDSPAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
    const auto mainOutput = layouts.getMainOutputChannelSet();
    const auto mainInput = layouts.getMainInputChannelSet();

    if (mainOutput != juce::AudioChannelSet::mono()
        && mainOutput != juce::AudioChannelSet::stereo())
        return false;

    if (mainInput != juce::AudioChannelSet::mono()
        && mainInput != juce::AudioChannelSet::stereo())
        return false;

    return true;
}

void LumenDSPAudioProcessor::processBlock(
    juce::AudioBuffer<float>& buffer,
    juce::MidiBuffer& midiMessages)
{
    juce::ignoreUnused(midiMessages);
    juce::ScopedNoDenormals noDenormals;

    const int totalNumInputChannels = getTotalNumInputChannels();
    const int totalNumOutputChannels = getTotalNumOutputChannels();

    for (int channel = totalNumInputChannels; channel < totalNumOutputChannels; ++channel)
        buffer.clear(channel, 0, buffer.getNumSamples());

    audioPipeline.setControlState(readControlState());
    audioPipeline.process(buffer);
}

juce::AudioProcessorEditor* LumenDSPAudioProcessor::createEditor()
{
    return new LumenDSPAudioProcessorEditor(*this);
}

bool LumenDSPAudioProcessor::hasEditor() const
{
    return true;
}

const juce::String LumenDSPAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool LumenDSPAudioProcessor::acceptsMidi() const
{
    return false;
}

bool LumenDSPAudioProcessor::producesMidi() const
{
    return false;
}

bool LumenDSPAudioProcessor::isMidiEffect() const
{
    return false;
}

double LumenDSPAudioProcessor::getTailLengthSeconds() const
{
    return defaultTailLengthSeconds;
}

int LumenDSPAudioProcessor::getNumPrograms()
{
    return 1;
}

int LumenDSPAudioProcessor::getCurrentProgram()
{
    return 0;
}

void LumenDSPAudioProcessor::setCurrentProgram(int index)
{
    juce::ignoreUnused(index);
}

const juce::String LumenDSPAudioProcessor::getProgramName(int index)
{
    juce::ignoreUnused(index);
    return {};
}

void LumenDSPAudioProcessor::changeProgramName(int index, const juce::String& newName)
{
    juce::ignoreUnused(index, newName);
}

void LumenDSPAudioProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    auto state = apvts.copyState();
    state.setProperty(
        namPathProperty,
        audioPipeline.getNamEngine().getLoadedModelFile().getFullPathName(),
        nullptr);
    state.setProperty(
        irPathProperty,
        audioPipeline.getIrConvolver().getLoadedFile().getFullPathName(),
        nullptr);

    std::unique_ptr<juce::XmlElement> xml(state.createXml());

    if (xml != nullptr)
        copyXmlToBinary(*xml, destData);
}

void LumenDSPAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));

    if (xmlState == nullptr)
        return;

    auto stateTree = juce::ValueTree::fromXml(*xmlState);

    if (!stateTree.isValid())
        return;

    apvts.replaceState(stateTree);
    restoreAttachedFilesFromState(stateTree);
}

juce::AudioProcessorValueTreeState& LumenDSPAudioProcessor::getValueTreeState() noexcept
{
    return apvts;
}

lumen::audio::AudioPipeline& LumenDSPAudioProcessor::getAudioPipeline() noexcept
{
    return audioPipeline;
}

lumen::presets::PresetManager& LumenDSPAudioProcessor::getPresetManager() noexcept
{
    return presetManager;
}

void LumenDSPAudioProcessor::requestNamLoad(const juce::File& modelFile)
{
    backgroundLoadPool.addJob([this, modelFile]() {
        audioPipeline.getNamEngine().loadModelAsync(modelFile);
    });
}

void LumenDSPAudioProcessor::requestIrLoad(const juce::File& irFile)
{
    backgroundLoadPool.addJob([this, irFile]() {
        audioPipeline.getIrConvolver().loadImpulseResponseAsync(irFile);
    });
}

bool LumenDSPAudioProcessor::applyPreset(const juce::String& presetName)
{
    lumen::presets::PresetData presetData;

    if (!presetManager.loadPreset(presetName, presetData))
        return false;

    if (presetData.parameterState.isValid())
        apvts.replaceState(presetData.parameterState);

    if (presetData.namModelPath.isNotEmpty())
    {
        const juce::File namFile(presetData.namModelPath);

        if (namFile.existsAsFile())
            requestNamLoad(namFile);
    }

    if (presetData.irPath.isNotEmpty())
    {
        const juce::File irFile(presetData.irPath);

        if (irFile.existsAsFile())
            requestIrLoad(irFile);
    }

    return true;
}

bool LumenDSPAudioProcessor::storePreset(const juce::String& presetName)
{
    return presetManager.savePreset(
        presetName,
        audioPipeline.getNamEngine().getLoadedModelFile(),
        audioPipeline.getIrConvolver().getLoadedFile());
}

void LumenDSPAudioProcessor::ensureFactoryContentReady()
{
    if (factoryBootstrapComplete)
        return;

    bootstrapFactoryExperience();
}

juce::String LumenDSPAudioProcessor::getFactoryStatusMessage() const
{
    return factoryStatusMessage;
}

juce::String LumenDSPAudioProcessor::getDefaultPresetName() const
{
    return defaultPresetName;
}

void LumenDSPAudioProcessor::bootstrapFactoryExperience()
{
    const auto applicationSupportRoot =
        juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory)
            .getChildFile("LumenDSP");
    applicationSupportRoot.createDirectory();

    const auto installResult = lumen::presets::FactoryContentInstaller::installIfNeeded(presetManager);
    factoryStatusMessage = installResult.statusMessage;
    defaultPresetName = installResult.defaultPresetName;

    const auto bundledRoot = lumen::presets::FactoryContentInstaller::findBundledFactoryRoot();
    applicationSupportRoot.getChildFile("factory_bootstrap.log")
        .replaceWithText(
            "status=" + installResult.statusMessage + "\n"
            + "bundled=" + bundledRoot.getFullPathName() + "\n"
            + "models=" + installResult.modelsDirectory.getFullPathName() + "\n"
            + "fresh=" + juce::String(installResult.performedFreshInstall ? "true" : "false") + "\n"
            + "executable="
            + juce::File::getSpecialLocation(juce::File::currentExecutableFile).getFullPathName()
            + "\n");

    if (installResult.installedOrAlreadyPresent)
    {
        applyPreset(defaultPresetName);
        presetManager.setCurrentPresetName(defaultPresetName);
    }

    factoryBootstrapComplete = true;
}

lumen::audio::PipelineControlState LumenDSPAudioProcessor::readControlState() const
{
    lumen::audio::PipelineControlState controlState;
    controlState.inputGainDb = apvts.getRawParameterValue(lumen::parameters::inputGainId)->load();
    controlState.outputLevelDb = apvts.getRawParameterValue(lumen::parameters::outputLevelId)->load();
    controlState.noiseGateThresholdDb =
        apvts.getRawParameterValue(lumen::parameters::noiseGateThresholdId)->load();
    controlState.noiseGateEnabled =
        apvts.getRawParameterValue(lumen::parameters::noiseGateEnabledId)->load() > 0.5f;
    controlState.bassGainDb = apvts.getRawParameterValue(lumen::parameters::bassGainId)->load();
    controlState.midGainDb = apvts.getRawParameterValue(lumen::parameters::midGainId)->load();
    controlState.trebleGainDb = apvts.getRawParameterValue(lumen::parameters::trebleGainId)->load();
    controlState.eqEnabled = apvts.getRawParameterValue(lumen::parameters::eqEnabledId)->load() > 0.5f;
    controlState.cabEnabled = apvts.getRawParameterValue(lumen::parameters::cabEnabledId)->load() > 0.5f;
    return controlState;
}

void LumenDSPAudioProcessor::restoreAttachedFilesFromState(const juce::ValueTree& stateTree)
{
    const auto namPath = stateTree.getProperty(namPathProperty).toString();
    const auto irPath = stateTree.getProperty(irPathProperty).toString();

    if (namPath.isNotEmpty())
    {
        const juce::File namFile(namPath);

        if (namFile.existsAsFile())
            requestNamLoad(namFile);
    }

    if (irPath.isNotEmpty())
    {
        const juce::File irFile(irPath);

        if (irFile.existsAsFile())
            requestIrLoad(irFile);
    }
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new LumenDSPAudioProcessor();
}
