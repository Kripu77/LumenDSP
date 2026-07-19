#include "presets/FactoryContentInstaller.h"
#include "parameters/ParameterIds.h"

#include <array>

namespace lumen::presets
{

namespace
{

void setParameterValue(
    juce::AudioProcessorValueTreeState& apvts,
    const juce::String& parameterId,
    float actualValue)
{
    if (auto* rangedParameter = dynamic_cast<juce::RangedAudioParameter*>(apvts.getParameter(parameterId)))
        rangedParameter->setValueNotifyingHost(rangedParameter->convertTo0to1(actualValue));
}

void setBoolParameter(
    juce::AudioProcessorValueTreeState& apvts,
    const juce::String& parameterId,
    bool isEnabled)
{
    if (auto* parameter = apvts.getParameter(parameterId))
        parameter->setValueNotifyingHost(isEnabled ? 1.0f : 0.0f);
}

void setChoiceParameter(
    juce::AudioProcessorValueTreeState& apvts,
    const juce::String& parameterId,
    int choiceIndex)
{
    if (auto* choice = dynamic_cast<juce::AudioParameterChoice*>(apvts.getParameter(parameterId)))
    {
        const auto clamped = juce::jlimit(0, juce::jmax(0, choice->choices.size() - 1), choiceIndex);
        choice->setValueNotifyingHost(choice->convertTo0to1(static_cast<float>(clamped)));
        return;
    }

    setParameterValue(apvts, parameterId, static_cast<float>(choiceIndex));
}

struct FactoryTone
{
    const char* name = nullptr;
    const char* category = nullptr;
    juce::StringArray tags;

    juce::File namFile;
    juce::File irFile;

    float inputGainDb = 0.0f;
    float noiseGateThresholdDb = -80.0f;
    bool noiseGateEnabled = true;

    bool compressorEnabled = false;
    float compressorThresholdDb = -18.0f;
    float compressorRatio = 4.0f;
    float compressorMix = 1.0f;

    bool driveEnabled = false;
    int driveMode = 0;
    float driveAmount = 0.35f;
    float driveTone = 0.5f;
    float driveLevel = 0.7f;

    float bassGainDb = 0.0f;
    float midGainDb = 0.0f;
    float trebleGainDb = 0.0f;
    bool eqEnabled = true;
    bool cabEnabled = true;

    bool delayEnabled = false;
    bool delaySync = false;
    int delayDivision = 0;
    float delayTimeMs = 380.0f;
    float delayFeedback = 0.25f;
    float delayMix = 0.2f;

    bool reverbEnabled = false;
    int reverbCharacter = 0;
    float reverbSize = 0.4f;
    float reverbDamping = 0.5f;
    float reverbMix = 0.18f;

    float outputLevelDb = -8.0f;
};

void resetMetronomeDefaults(juce::AudioProcessorValueTreeState& apvts)
{
    setBoolParameter(apvts, parameters::metronomeEnabledId, false);
    setParameterValue(apvts, parameters::metronomeBpmId, parameters::ranges::metronomeBpmDefault);
    setParameterValue(apvts, parameters::metronomeVolumeId, parameters::ranges::metronomeVolumeDefault);
}

void applyFactoryTone(juce::AudioProcessorValueTreeState& apvts, const FactoryTone& tone)
{
    setParameterValue(apvts, parameters::inputGainId, tone.inputGainDb);
    setParameterValue(apvts, parameters::noiseGateThresholdId, tone.noiseGateThresholdDb);
    setBoolParameter(apvts, parameters::noiseGateEnabledId, tone.noiseGateEnabled);

    setBoolParameter(apvts, parameters::compressorEnabledId, tone.compressorEnabled);
    setParameterValue(apvts, parameters::compressorThresholdId, tone.compressorThresholdDb);
    setParameterValue(apvts, parameters::compressorRatioId, tone.compressorRatio);
    setParameterValue(apvts, parameters::compressorMixId, tone.compressorMix);

    setBoolParameter(apvts, parameters::driveEnabledId, tone.driveEnabled);
    setChoiceParameter(apvts, parameters::driveModeId, tone.driveMode);
    setParameterValue(apvts, parameters::driveAmountId, tone.driveAmount);
    setParameterValue(apvts, parameters::driveToneId, tone.driveTone);
    setParameterValue(apvts, parameters::driveLevelId, tone.driveLevel);

    setParameterValue(apvts, parameters::bassGainId, tone.bassGainDb);
    setParameterValue(apvts, parameters::midGainId, tone.midGainDb);
    setParameterValue(apvts, parameters::trebleGainId, tone.trebleGainDb);
    setBoolParameter(apvts, parameters::eqEnabledId, tone.eqEnabled);
    setBoolParameter(apvts, parameters::cabEnabledId, tone.cabEnabled);

    setBoolParameter(apvts, parameters::delayEnabledId, tone.delayEnabled);
    setBoolParameter(apvts, parameters::delaySyncId, tone.delaySync);
    setChoiceParameter(apvts, parameters::delayDivisionId, tone.delayDivision);
    setParameterValue(apvts, parameters::delayTimeId, tone.delayTimeMs);
    setParameterValue(apvts, parameters::delayFeedbackId, tone.delayFeedback);
    setParameterValue(apvts, parameters::delayMixId, tone.delayMix);

    setBoolParameter(apvts, parameters::reverbEnabledId, tone.reverbEnabled);
    setChoiceParameter(apvts, parameters::reverbCharacterId, tone.reverbCharacter);
    setParameterValue(apvts, parameters::reverbSizeId, tone.reverbSize);
    setParameterValue(apvts, parameters::reverbDampingId, tone.reverbDamping);
    setParameterValue(apvts, parameters::reverbMixId, tone.reverbMix);

    setParameterValue(apvts, parameters::outputLevelId, tone.outputLevelDb);
    resetMetronomeDefaults(apvts);
}

bool saveFactoryTone(PresetManager& presetManager, const FactoryTone& tone)
{
    applyFactoryTone(presetManager.getValueTreeState(), tone);
    return presetManager.savePreset(
        tone.name,
        tone.namFile,
        tone.irFile,
        tone.category,
        tone.tags);
}

} // namespace

juce::File FactoryContentInstaller::getUserContentRoot()
{
    return juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory)
        .getChildFile(applicationFolderName)
        .getChildFile(contentFolderName);
}

juce::File FactoryContentInstaller::getUserModelsDirectory()
{
    return getUserContentRoot().getChildFile(modelsFolderName);
}

juce::File FactoryContentInstaller::getUserIrsDirectory()
{
    return getUserContentRoot().getChildFile(irsFolderName);
}

juce::File FactoryContentInstaller::getMarkerFile()
{
    return getUserContentRoot().getChildFile(installMarkerFileName);
}

juce::File FactoryContentInstaller::findBundledFactoryRoot()
{
    const auto executableFile = juce::File::getSpecialLocation(juce::File::currentExecutableFile);
    juce::Array<juce::File> candidates;

#if JUCE_MAC
    candidates.add(executableFile.getParentDirectory()
                       .getParentDirectory()
                       .getChildFile("Resources")
                       .getChildFile(contentFolderName));
    candidates.add(executableFile.getParentDirectory()
                       .getParentDirectory()
                       .getParentDirectory()
                       .getChildFile("Contents")
                       .getChildFile("Resources")
                       .getChildFile(contentFolderName));
#endif

    candidates.add(executableFile.getParentDirectory().getChildFile(contentFolderName));
    candidates.add(executableFile.getParentDirectory().getParentDirectory().getChildFile(contentFolderName));
    candidates.add(executableFile.getParentDirectory()
                       .getParentDirectory()
                       .getChildFile("Resources")
                       .getChildFile(contentFolderName));
    candidates.add(juce::File::getCurrentWorkingDirectory()
                       .getChildFile("Resources")
                       .getChildFile(contentFolderName));
    candidates.add(juce::File::getCurrentWorkingDirectory()
                       .getChildFile("..")
                       .getChildFile("Resources")
                       .getChildFile(contentFolderName));
    candidates.add(juce::File::getCurrentWorkingDirectory()
                       .getChildFile("FactoryContent"));

    for (const auto& candidate : candidates)
    {
        const auto modelsDirectory = candidate.getChildFile(modelsFolderName);
        if (candidate.isDirectory() && modelsDirectory.isDirectory())
            return candidate;
    }

    return {};
}

FactoryContentInstaller::InstallResult FactoryContentInstaller::installIfNeeded(PresetManager& presetManager)
{
    InstallResult result;
    result.defaultPresetName = defaultPresetName;
    result.modelsDirectory = getUserModelsDirectory();
    result.irsDirectory = getUserIrsDirectory();

    const bool markerPresent = getMarkerFile().existsAsFile();
    const bool legacyMarkerPresent =
        getUserContentRoot().getChildFile(legacyMarkerFileName).existsAsFile();
    const bool modelsPresent = result.modelsDirectory.getNumberOfChildFiles(
                                   juce::File::findFiles, "*.nam")
                               > 0;
    const bool irsPresent = result.irsDirectory.getNumberOfChildFiles(
                                juce::File::findFiles, "*.wav")
                            > 0;
    const bool presetsPresent = presetManager.getPresetNames().contains(defaultPresetName);
    const bool ambientPresent = presetManager.getPresetNames().contains("06 Ambient Swell");

    if (markerPresent && modelsPresent && irsPresent && presetsPresent && ambientPresent)
    {
        result.installedOrAlreadyPresent = true;
        result.statusMessage = "Factory content already installed.";
        return result;
    }

    const auto bundledRoot = findBundledFactoryRoot();

    if (!bundledRoot.isDirectory())
    {
        result.statusMessage = "Bundled factory content was not found next to the application.";
        return result;
    }

    getUserContentRoot().createDirectory();
    result.modelsDirectory.createDirectory();
    result.irsDirectory.createDirectory();

    const auto bundledModels = bundledRoot.getChildFile(modelsFolderName);
    const auto bundledIrs = bundledRoot.getChildFile(irsFolderName);

    if (!copyDirectoryRecursive(bundledModels, result.modelsDirectory))
    {
        result.statusMessage = "Failed to install factory NAM models.";
        return result;
    }

    if (!copyDirectoryRecursive(bundledIrs, result.irsDirectory))
    {
        result.statusMessage = "Failed to install factory cabinet IRs.";
        return result;
    }

    if (!writeFactoryPresets(presetManager, result.modelsDirectory, result.irsDirectory))
    {
        result.statusMessage = "Factory files installed, but preset creation failed.";
        return result;
    }

    getMarkerFile().replaceWithText(
        "LumenDSP factory content v" + juce::String(factoryContentVersion) + "\n");
    getUserContentRoot().getChildFile(legacyMarkerFileName).deleteFile();

    result.installedOrAlreadyPresent = true;
    result.performedFreshInstall = !legacyMarkerPresent && !presetsPresent;
    result.upgradedExistingInstall = legacyMarkerPresent || (presetsPresent && !ambientPresent);
    result.statusMessage = result.upgradedExistingInstall
                               ? "Factory fusion tones upgraded (full FX chain)."
                               : "Factory fusion tones and cab IRs installed.";
    return result;
}

bool FactoryContentInstaller::writeFactoryPresets(
    PresetManager& presetManager,
    const juce::File& modelsDirectory,
    const juce::File& irsDirectory)
{
    const auto twinClean = modelsDirectory.getChildFile("TwinBright_Clean.nam");
    const auto twinAiry = modelsDirectory.getChildFile("TwinBright_Airy.nam");
    const auto bugClean = modelsDirectory.getChildFile("Bug333_Clean.nam");
    const auto bugCleanCab = modelsDirectory.getChildFile("Bug333_CleanCab.nam");
    const auto jcmClean = modelsDirectory.getChildFile("JCM2000_Clean.nam");

    const auto glassIr = irsDirectory.getChildFile("Lumen_Glass_212.wav");
    const auto smoothIr = irsDirectory.getChildFile("Lumen_Smooth_Lead_412.wav");
    const auto crunchIr = irsDirectory.getChildFile("Lumen_Tight_Crunch_112.wav");

    if (!twinClean.existsAsFile() || !twinAiry.existsAsFile() || !bugClean.existsAsFile()
        || !bugCleanCab.existsAsFile() || !jcmClean.existsAsFile() || !glassIr.existsAsFile()
        || !smoothIr.existsAsFile() || !crunchIr.existsAsFile())
        return false;

    constexpr int driveSoft = 0;
    constexpr int driveTube = 2;
    constexpr int driveBoost = 3;
    constexpr int reverbRoom = 0;
    constexpr int reverbPlate = 2;
    constexpr int reverbAmbient = 3;

    std::array<FactoryTone, 6> tones {};

    {
        auto& tone = tones[0];
        tone.name = "01 Glass Clean";
        tone.category = "Clean";
        tone.tags = { "glass", "fusion", "factory" };
        tone.namFile = twinClean;
        tone.irFile = glassIr;
        tone.inputGainDb = 0.0f;
        tone.noiseGateThresholdDb = -84.0f;
        tone.compressorEnabled = true;
        tone.compressorThresholdDb = -22.0f;
        tone.compressorRatio = 2.5f;
        tone.compressorMix = 0.55f;
        tone.bassGainDb = -1.0f;
        tone.midGainDb = 1.5f;
        tone.trebleGainDb = 2.5f;
        tone.reverbEnabled = true;
        tone.reverbCharacter = reverbRoom;
        tone.reverbSize = 0.32f;
        tone.reverbDamping = 0.45f;
        tone.reverbMix = 0.14f;
        tone.outputLevelDb = -8.0f;
    }

    {
        auto& tone = tones[1];
        tone.name = "02 Airy Clean";
        tone.category = "Clean";
        tone.tags = { "airy", "fusion", "factory" };
        tone.namFile = twinAiry;
        tone.irFile = glassIr;
        tone.inputGainDb = 1.0f;
        tone.noiseGateThresholdDb = -82.0f;
        tone.compressorEnabled = true;
        tone.compressorThresholdDb = -20.0f;
        tone.compressorRatio = 2.0f;
        tone.compressorMix = 0.45f;
        tone.bassGainDb = -0.5f;
        tone.midGainDb = 0.8f;
        tone.trebleGainDb = 3.2f;
        tone.delayEnabled = true;
        tone.delayTimeMs = 420.0f;
        tone.delayFeedback = 0.18f;
        tone.delayMix = 0.12f;
        tone.reverbEnabled = true;
        tone.reverbCharacter = reverbAmbient;
        tone.reverbSize = 0.55f;
        tone.reverbDamping = 0.35f;
        tone.reverbMix = 0.22f;
        tone.outputLevelDb = -8.0f;
    }

    {
        auto& tone = tones[2];
        tone.name = "03 Smooth Lead";
        tone.category = "Lead";
        tone.tags = { "lead", "sustain", "factory" };
        tone.namFile = bugClean;
        tone.irFile = smoothIr;
        tone.inputGainDb = 3.5f;
        tone.noiseGateThresholdDb = -74.0f;
        tone.compressorEnabled = true;
        tone.compressorThresholdDb = -16.0f;
        tone.compressorRatio = 4.5f;
        tone.compressorMix = 0.85f;
        tone.driveEnabled = true;
        tone.driveMode = driveTube;
        tone.driveAmount = 0.28f;
        tone.driveTone = 0.55f;
        tone.driveLevel = 0.72f;
        tone.bassGainDb = 0.0f;
        tone.midGainDb = 2.8f;
        tone.trebleGainDb = 1.2f;
        tone.delayEnabled = true;
        tone.delayTimeMs = 460.0f;
        tone.delayFeedback = 0.32f;
        tone.delayMix = 0.2f;
        tone.reverbEnabled = true;
        tone.reverbCharacter = reverbPlate;
        tone.reverbSize = 0.48f;
        tone.reverbDamping = 0.4f;
        tone.reverbMix = 0.2f;
        tone.outputLevelDb = -10.0f;
    }

    {
        auto& tone = tones[3];
        tone.name = "04 Full Rig Clean";
        tone.category = "Clean";
        tone.tags = { "full-rig", "factory" };
        tone.namFile = bugCleanCab;
        tone.irFile = juce::File();
        tone.inputGainDb = 0.0f;
        tone.noiseGateThresholdDb = -80.0f;
        tone.compressorEnabled = true;
        tone.compressorThresholdDb = -20.0f;
        tone.compressorRatio = 2.2f;
        tone.compressorMix = 0.4f;
        tone.bassGainDb = 0.0f;
        tone.midGainDb = 0.5f;
        tone.trebleGainDb = 1.0f;
        tone.cabEnabled = false;
        tone.reverbEnabled = true;
        tone.reverbCharacter = reverbRoom;
        tone.reverbSize = 0.28f;
        tone.reverbDamping = 0.55f;
        tone.reverbMix = 0.1f;
        tone.outputLevelDb = -6.0f;
    }

    {
        auto& tone = tones[4];
        tone.name = "05 Light Crunch";
        tone.category = "Crunch";
        tone.tags = { "crunch", "rhythm", "factory" };
        tone.namFile = jcmClean;
        tone.irFile = crunchIr;
        tone.inputGainDb = 5.0f;
        tone.noiseGateThresholdDb = -70.0f;
        tone.compressorEnabled = true;
        tone.compressorThresholdDb = -14.0f;
        tone.compressorRatio = 3.5f;
        tone.compressorMix = 0.5f;
        tone.driveEnabled = true;
        tone.driveMode = driveSoft;
        tone.driveAmount = 0.42f;
        tone.driveTone = 0.48f;
        tone.driveLevel = 0.68f;
        tone.bassGainDb = -2.0f;
        tone.midGainDb = 3.0f;
        tone.trebleGainDb = 0.5f;
        tone.reverbEnabled = true;
        tone.reverbCharacter = reverbRoom;
        tone.reverbSize = 0.25f;
        tone.reverbDamping = 0.6f;
        tone.reverbMix = 0.08f;
        tone.outputLevelDb = -9.0f;
    }

    {
        auto& tone = tones[5];
        tone.name = "06 Ambient Swell";
        tone.category = "Ambient";
        tone.tags = { "ambient", "swell", "fusion", "factory" };
        tone.namFile = twinAiry;
        tone.irFile = smoothIr;
        tone.inputGainDb = 2.0f;
        tone.noiseGateThresholdDb = -88.0f;
        tone.compressorEnabled = true;
        tone.compressorThresholdDb = -24.0f;
        tone.compressorRatio = 3.0f;
        tone.compressorMix = 0.7f;
        tone.driveEnabled = true;
        tone.driveMode = driveBoost;
        tone.driveAmount = 0.18f;
        tone.driveTone = 0.6f;
        tone.driveLevel = 0.75f;
        tone.bassGainDb = -1.5f;
        tone.midGainDb = 1.0f;
        tone.trebleGainDb = 2.0f;
        tone.delayEnabled = true;
        tone.delaySync = false;
        tone.delayTimeMs = 520.0f;
        tone.delayFeedback = 0.42f;
        tone.delayMix = 0.28f;
        tone.reverbEnabled = true;
        tone.reverbCharacter = reverbAmbient;
        tone.reverbSize = 0.72f;
        tone.reverbDamping = 0.3f;
        tone.reverbMix = 0.34f;
        tone.outputLevelDb = -11.0f;
    }

    bool allSucceeded = true;
    for (const auto& tone : tones)
        allSucceeded &= saveFactoryTone(presetManager, tone);

    return allSucceeded;
}

bool FactoryContentInstaller::copyDirectoryRecursive(const juce::File& source, const juce::File& destination)
{
    if (!source.isDirectory())
        return false;

    destination.createDirectory();
    const auto children = source.findChildFiles(juce::File::findFilesAndDirectories, false);

    for (const auto& child : children)
    {
        const auto target = destination.getChildFile(child.getFileName());

        if (child.isDirectory())
        {
            if (!copyDirectoryRecursive(child, target))
                return false;
        }
        else
        {
            if (target.existsAsFile())
                target.deleteFile();

            if (!child.copyFileTo(target))
                return false;
        }
    }

    return true;
}

} // namespace lumen::presets
