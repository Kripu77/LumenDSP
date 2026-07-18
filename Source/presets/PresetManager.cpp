#include "presets/PresetManager.h"

namespace lumen::presets
{

PresetManager::PresetManager(juce::AudioProcessorValueTreeState& valueTreeState)
    : apvts(valueTreeState)
{
    getPresetDirectory().createDirectory();
}

juce::AudioProcessorValueTreeState& PresetManager::getValueTreeState() noexcept
{
    return apvts;
}

juce::File PresetManager::getPresetDirectory() const
{
    return juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory)
        .getChildFile(applicationFolderName)
        .getChildFile(presetsFolderName);
}

juce::StringArray PresetManager::getPresetNames() const
{
    juce::StringArray presetNames;
    const auto presetFiles = getPresetDirectory().findChildFiles(
        juce::File::findFiles,
        false,
        juce::String("*") + presetFileExtension);

    for (const auto& presetFile : presetFiles)
        presetNames.add(presetFile.getFileNameWithoutExtension());

    presetNames.sort(true);
    return presetNames;
}

bool PresetManager::savePreset(
    const juce::String& presetName,
    const juce::File& namModelFile,
    const juce::File& irFile)
{
    const auto sanitizedName = sanitizePresetName(presetName);

    if (sanitizedName.isEmpty())
        return false;

    juce::ValueTree presetTree(rootXmlTag);
    presetTree.setProperty(nameAttribute, sanitizedName, nullptr);
    presetTree.setProperty(namPathAttribute, namModelFile.getFullPathName(), nullptr);
    presetTree.setProperty(irPathAttribute, irFile.getFullPathName(), nullptr);

    const auto parameterState = apvts.copyState();
    juce::ValueTree parametersTree(parametersChildTag);
    parametersTree.copyPropertiesAndChildrenFrom(parameterState, nullptr);
    presetTree.addChild(parametersTree, -1, nullptr);

    const auto presetFile = resolvePresetFile(sanitizedName);
    std::unique_ptr<juce::XmlElement> xml(presetTree.createXml());

    if (xml == nullptr)
        return false;

    if (!xml->writeTo(presetFile))
        return false;

    currentPresetName = sanitizedName;
    return true;
}

bool PresetManager::loadPreset(const juce::String& presetName, PresetData& presetDataOut)
{
    const auto sanitizedName = sanitizePresetName(presetName);
    const auto presetFile = resolvePresetFile(sanitizedName);

    if (!presetFile.existsAsFile())
        return false;

    auto xml = juce::XmlDocument::parse(presetFile);

    if (xml == nullptr)
        return false;

    auto presetTree = juce::ValueTree::fromXml(*xml);

    if (!presetTree.hasType(rootXmlTag))
        return false;

    presetDataOut.name = presetTree.getProperty(nameAttribute).toString();
    presetDataOut.namModelPath = presetTree.getProperty(namPathAttribute).toString();
    presetDataOut.irPath = presetTree.getProperty(irPathAttribute).toString();

    auto parametersTree = presetTree.getChildWithName(parametersChildTag);

    if (!parametersTree.isValid())
        return false;

    presetDataOut.parameterState = parametersTree;
    currentPresetName = presetDataOut.name;
    return true;
}

bool PresetManager::deletePreset(const juce::String& presetName)
{
    const auto presetFile = resolvePresetFile(sanitizePresetName(presetName));

    if (!presetFile.existsAsFile())
        return false;

    const bool removed = presetFile.deleteFile();

    if (removed && currentPresetName == sanitizePresetName(presetName))
        currentPresetName.clear();

    return removed;
}

juce::String PresetManager::getCurrentPresetName() const
{
    return currentPresetName;
}

void PresetManager::setCurrentPresetName(const juce::String& presetName)
{
    currentPresetName = sanitizePresetName(presetName);
}

juce::File PresetManager::resolvePresetFile(const juce::String& presetName) const
{
    return getPresetDirectory().getChildFile(presetName + presetFileExtension);
}

juce::String PresetManager::sanitizePresetName(const juce::String& presetName) const
{
    auto sanitized = presetName.trim();
    sanitized = sanitized.replaceCharacter('/', '-');
    sanitized = sanitized.replaceCharacter('\\', '-');
    sanitized = sanitized.replaceCharacter(':', '-');
    return sanitized;
}

} // namespace lumen::presets
