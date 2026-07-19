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

    // driveMode: 0 Soft, 1 Hard, 2 Tube, 3 Boost
    // delayDivision: 0 1/4, 1 1/8, 2 1/8D, 3 1/16, 4 1/4D, 5 1/2
    // reverbCharacter: 0 Room, 1 Hall, 2 Plate, 3 Ambient

    std::array<FactoryTone, 6> tones {};

    // 01 — glassy Twin clean, light room air, soft leveling
    {
        auto& t = tones[0];
        t.name = "01 Glass Clean";
        t.category = "Clean";
        t.tags = { "glass", "fusion", "factory" };
        t.namFile = twinClean;
        t.irFile = glassIr;
        t.inputGainDb = 0.0f;
        t.noiseGateThresholdDb = -84.0f;
        t.compressorEnabled = true;
        t.compressorThresholdDb = -22.0f;
        t.compressorRatio = 2.5f;
        t.compressorMix = 0.55f;
        t.bassGainDb = -1.0f;
        t.midGainDb = 1.5f;
        t.trebleGainDb = 2.5f;
        t.reverbEnabled = true;
        t.reverbCharacter = 0;
        t.reverbSize = 0.32f;
        t.reverbDamping = 0.45f;
        t.reverbMix = 0.14f;
        t.outputLevelDb = -8.0f;
    }

    // 02 — brighter Twin, more air and ambient bloom
    {
        auto& t = tones[1];
        t.name = "02 Airy Clean";
        t.category = "Clean";
        t.tags = { "airy", "fusion", "factory" };
        t.namFile = twinAiry;
        t.irFile = glassIr;
        t.inputGainDb = 1.0f;
        t.noiseGateThresholdDb = -82.0f;
        t.compressorEnabled = true;
        t.compressorThresholdDb = -20.0f;
        t.compressorRatio = 2.0f;
        t.compressorMix = 0.45f;
        t.bassGainDb = -0.5f;
        t.midGainDb = 0.8f;
        t.trebleGainDb = 3.2f;
        t.delayEnabled = true;
        t.delayTimeMs = 420.0f;
        t.delayFeedback = 0.18f;
        t.delayMix = 0.12f;
        t.reverbEnabled = true;
        t.reverbCharacter = 3;
        t.reverbSize = 0.55f;
        t.reverbDamping = 0.35f;
        t.reverbMix = 0.22f;
        t.outputLevelDb = -8.0f;
    }

    // 03 — sustained lead: tube push, plate, dotted delay
    {
        auto& t = tones[2];
        t.name = "03 Smooth Lead";
        t.category = "Lead";
        t.tags = { "lead", "sustain", "factory" };
        t.namFile = bugClean;
        t.irFile = smoothIr;
        t.inputGainDb = 3.5f;
        t.noiseGateThresholdDb = -74.0f;
        t.compressorEnabled = true;
        t.compressorThresholdDb = -16.0f;
        t.compressorRatio = 4.5f;
        t.compressorMix = 0.85f;
        t.driveEnabled = true;
        t.driveMode = 2; // Tube
        t.driveAmount = 0.28f;
        t.driveTone = 0.55f;
        t.driveLevel = 0.72f;
        t.bassGainDb = 0.0f;
        t.midGainDb = 2.8f;
        t.trebleGainDb = 1.2f;
        t.delayEnabled = true;
        t.delayTimeMs = 460.0f;
        t.delayFeedback = 0.32f;
        t.delayMix = 0.2f;
        t.reverbEnabled = true;
        t.reverbCharacter = 2; // Plate
        t.reverbSize = 0.48f;
        t.reverbDamping = 0.4f;
        t.reverbMix = 0.2f;
        t.outputLevelDb = -10.0f;
    }

    // 04 — cab baked into capture; keep path dry and clear
    {
        auto& t = tones[3];
        t.name = "04 Full Rig Clean";
        t.category = "Clean";
        t.tags = { "full-rig", "factory" };
        t.namFile = bugCleanCab;
        t.irFile = juce::File();
        t.inputGainDb = 0.0f;
        t.noiseGateThresholdDb = -80.0f;
        t.compressorEnabled = true;
        t.compressorThresholdDb = -20.0f;
        t.compressorRatio = 2.2f;
        t.compressorMix = 0.4f;
        t.bassGainDb = 0.0f;
        t.midGainDb = 0.5f;
        t.trebleGainDb = 1.0f;
        t.cabEnabled = false;
        t.reverbEnabled = true;
        t.reverbCharacter = 0;
        t.reverbSize = 0.28f;
        t.reverbDamping = 0.55f;
        t.reverbMix = 0.1f;
        t.outputLevelDb = -6.0f;
    }

    // 05 — tight rhythm crunch
    {
        auto& t = tones[4];
        t.name = "05 Light Crunch";
        t.category = "Crunch";
        t.tags = { "crunch", "rhythm", "factory" };
        t.namFile = jcmClean;
        t.irFile = crunchIr;
        t.inputGainDb = 5.0f;
        t.noiseGateThresholdDb = -70.0f;
        t.compressorEnabled = true;
        t.compressorThresholdDb = -14.0f;
        t.compressorRatio = 3.5f;
        t.compressorMix = 0.5f;
        t.driveEnabled = true;
        t.driveMode = 0; // Soft
        t.driveAmount = 0.42f;
        t.driveTone = 0.48f;
        t.driveLevel = 0.68f;
        t.bassGainDb = -2.0f;
        t.midGainDb = 3.0f;
        t.trebleGainDb = 0.5f;
        t.reverbEnabled = true;
        t.reverbCharacter = 0;
        t.reverbSize = 0.25f;
        t.reverbDamping = 0.6f;
        t.reverbMix = 0.08f;
        t.outputLevelDb = -9.0f;
    }

    // 06 — ambient fusion pad: soft clean + long trails
    {
        auto& t = tones[5];
        t.name = "06 Ambient Swell";
        t.category = "Ambient";
        t.tags = { "ambient", "swell", "fusion", "factory" };
        t.namFile = twinAiry;
        t.irFile = smoothIr;
        t.inputGainDb = 2.0f;
        t.noiseGateThresholdDb = -88.0f;
        t.compressorEnabled = true;
        t.compressorThresholdDb = -24.0f;
        t.compressorRatio = 3.0f;
        t.compressorMix = 0.7f;
        t.driveEnabled = true;
        t.driveMode = 3; // Boost
        t.driveAmount = 0.18f;
        t.driveTone = 0.6f;
        t.driveLevel = 0.75f;
        t.bassGainDb = -1.5f;
        t.midGainDb = 1.0f;
        t.trebleGainDb = 2.0f;
        t.delayEnabled = true;
        t.delaySync = false;
        t.delayTimeMs = 520.0f;
        t.delayFeedback = 0.42f;
        t.delayMix = 0.28f;
        t.reverbEnabled = true;
        t.reverbCharacter = 3; // Ambient
        t.reverbSize = 0.72f;
        t.reverbDamping = 0.3f;
        t.reverbMix = 0.34f;
        t.outputLevelDb = -11.0f;
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
