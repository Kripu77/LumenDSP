#include "presets/FactoryContentInstaller.h"
#include "parameters/ParameterIds.h"

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

void applyControlValues(
    juce::AudioProcessorValueTreeState& apvts,
    float inputGainDb,
    float noiseGateThresholdDb,
    bool noiseGateEnabled,
    float bassGainDb,
    float midGainDb,
    float trebleGainDb,
    bool eqEnabled,
    bool cabEnabled,
    float outputLevelDb)
{
    setParameterValue(apvts, parameters::inputGainId, inputGainDb);
    setParameterValue(apvts, parameters::noiseGateThresholdId, noiseGateThresholdDb);
    setBoolParameter(apvts, parameters::noiseGateEnabledId, noiseGateEnabled);
    setParameterValue(apvts, parameters::bassGainId, bassGainDb);
    setParameterValue(apvts, parameters::midGainId, midGainDb);
    setParameterValue(apvts, parameters::trebleGainId, trebleGainDb);
    setBoolParameter(apvts, parameters::eqEnabledId, eqEnabled);
    setBoolParameter(apvts, parameters::cabEnabledId, cabEnabled);
    setParameterValue(apvts, parameters::outputLevelId, outputLevelDb);
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
    const bool modelsPresent = result.modelsDirectory.getNumberOfChildFiles(
                                   juce::File::findFiles, "*.nam")
                               > 0;
    const bool irsPresent = result.irsDirectory.getNumberOfChildFiles(
                                juce::File::findFiles, "*.wav")
                            > 0;
    const bool presetsPresent = presetManager.getPresetNames().contains(defaultPresetName);

    if (markerPresent && modelsPresent && irsPresent && presetsPresent)
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

    getMarkerFile().replaceWithText("LumenDSP factory content v1\n");
    result.installedOrAlreadyPresent = true;
    result.performedFreshInstall = true;
    result.statusMessage = "Factory fusion tones and cab IRs installed.";
    return result;
}

bool FactoryContentInstaller::writeFactoryPresets(
    PresetManager& presetManager,
    const juce::File& modelsDirectory,
    const juce::File& irsDirectory)
{
    auto& apvts = presetManager.getValueTreeState();

    const auto twinClean = modelsDirectory.getChildFile("TwinBright_Clean.nam");
    const auto twinAiry = modelsDirectory.getChildFile("TwinBright_Airy.nam");
    const auto bugClean = modelsDirectory.getChildFile("Bug333_Clean.nam");
    const auto bugCleanCab = modelsDirectory.getChildFile("Bug333_CleanCab.nam");
    const auto jcmClean = modelsDirectory.getChildFile("JCM2000_Clean.nam");

    const auto glassIr = irsDirectory.getChildFile("Lumen_Glass_212.wav");
    const auto smoothIr = irsDirectory.getChildFile("Lumen_Smooth_Lead_412.wav");
    const auto crunchIr = irsDirectory.getChildFile("Lumen_Tight_Crunch_112.wav");

    if (!twinClean.existsAsFile() || !glassIr.existsAsFile())
        return false;

    bool allSucceeded = true;

    applyControlValues(apvts, 0.0f, -82.0f, true, -1.0f, 1.5f, 2.5f, true, true, -8.0f);
    allSucceeded &= presetManager.savePreset(
        "01 Glass Clean", twinClean, glassIr, "Clean", { "glass", "fusion", "factory" });

    applyControlValues(apvts, 1.0f, -80.0f, true, -0.5f, 1.0f, 3.0f, true, true, -8.0f);
    allSucceeded &= presetManager.savePreset(
        "02 Airy Clean", twinAiry, glassIr, "Clean", { "airy", "fusion", "factory" });

    applyControlValues(apvts, 4.0f, -76.0f, true, 0.0f, 2.5f, 1.5f, true, true, -10.0f);
    allSucceeded &= presetManager.savePreset(
        "03 Smooth Lead", bugClean, smoothIr, "Lead", { "lead", "sustain", "factory" });

    applyControlValues(apvts, 0.0f, -80.0f, true, 0.0f, 0.5f, 1.0f, true, false, -6.0f);
    allSucceeded &= presetManager.savePreset(
        "04 Full Rig Clean", bugCleanCab, juce::File(), "Clean", { "full-rig", "factory" });

    applyControlValues(apvts, 6.0f, -72.0f, true, -2.0f, 3.0f, 0.5f, true, true, -9.0f);
    allSucceeded &= presetManager.savePreset(
        "05 Light Crunch", jcmClean, crunchIr, "Crunch", { "crunch", "rhythm", "factory" });

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
