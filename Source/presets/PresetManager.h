#pragma once

#include <JuceHeader.h>

namespace lumen::presets
{

struct PresetData
{
    juce::String name;
    juce::String namModelPath;
    juce::String irPath;
    juce::ValueTree parameterState;
};

class PresetManager
{
public:
    explicit PresetManager(juce::AudioProcessorValueTreeState& valueTreeState);

    juce::File getPresetDirectory() const;
    juce::StringArray getPresetNames() const;
    bool savePreset(const juce::String& presetName, const juce::File& namModelFile, const juce::File& irFile);
    bool loadPreset(const juce::String& presetName, PresetData& presetDataOut);
    bool deletePreset(const juce::String& presetName);
    juce::String getCurrentPresetName() const;
    void setCurrentPresetName(const juce::String& presetName);

private:
    static constexpr const char* presetFileExtension = ".lumenpreset";
    static constexpr const char* applicationFolderName = "LumenDSP";
    static constexpr const char* presetsFolderName = "Presets";
    static constexpr const char* rootXmlTag = "LumenDSPPreset";
    static constexpr const char* nameAttribute = "name";
    static constexpr const char* namPathAttribute = "namModelPath";
    static constexpr const char* irPathAttribute = "irPath";
    static constexpr const char* parametersChildTag = "Parameters";

    juce::File resolvePresetFile(const juce::String& presetName) const;
    juce::String sanitizePresetName(const juce::String& presetName) const;

    juce::AudioProcessorValueTreeState& apvts;
    juce::String currentPresetName;
};

} // namespace lumen::presets
