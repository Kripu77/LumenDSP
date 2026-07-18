#pragma once

#include <JuceHeader.h>

namespace lumen::presets
{

struct PresetData
{
    juce::String name;
    juce::String category;
    juce::StringArray tags;
    juce::String namModelPath;
    juce::String irPath;
    juce::ValueTree parameterState;
    bool favorite = false;
};

struct PresetSummary
{
    juce::String name;
    juce::String category;
    juce::StringArray tags;
    bool favorite = false;
};

class PresetManager
{
public:
    explicit PresetManager(juce::AudioProcessorValueTreeState& valueTreeState);

    juce::AudioProcessorValueTreeState& getValueTreeState() noexcept;
    juce::File getPresetDirectory() const;
    juce::StringArray getPresetNames() const;
    juce::Array<PresetSummary> getPresetSummaries() const;
    juce::StringArray getCategories() const;

    bool savePreset(
        const juce::String& presetName,
        const juce::File& namModelFile,
        const juce::File& irFile,
        const juce::String& category = {},
        const juce::StringArray& tags = {});

    bool loadPreset(const juce::String& presetName, PresetData& presetDataOut);
    bool deletePreset(const juce::String& presetName);
    bool renamePreset(const juce::String& oldName, const juce::String& newName);
    bool toggleFavorite(const juce::String& presetName);
    bool exportPreset(const juce::String& presetName, const juce::File& destinationFile) const;
    bool importPreset(const juce::File& sourceFile, juce::String& errorOut, juce::String* importedNameOut = nullptr);

    juce::String getCurrentPresetName() const;
    void setCurrentPresetName(const juce::String& presetName);

private:
    static constexpr const char* presetFileExtension = ".lumenpreset";
    static constexpr const char* applicationFolderName = "LumenDSP";
    static constexpr const char* presetsFolderName = "Presets";
    static constexpr const char* favoritesFileName = "preset-favorites.json";
    static constexpr const char* rootXmlTag = "LumenDSPPreset";
    static constexpr const char* nameAttribute = "name";
    static constexpr const char* categoryAttribute = "category";
    static constexpr const char* tagsAttribute = "tags";
    static constexpr const char* namPathAttribute = "namModelPath";
    static constexpr const char* irPathAttribute = "irPath";
    static constexpr const char* parametersChildTag = "Parameters";

    juce::File resolvePresetFile(const juce::String& presetName) const;
    juce::String sanitizePresetName(const juce::String& presetName) const;
    juce::File favoritesFile() const;
    void loadFavorites();
    void saveFavorites() const;
    static juce::String tagsToString(const juce::StringArray& tags);
    static juce::StringArray tagsFromString(const juce::String& tags);
    static juce::String inferCategory(const juce::String& presetName);

    juce::AudioProcessorValueTreeState& apvts;
    juce::String currentPresetName;
    juce::StringArray favoriteNames;
};

} // namespace lumen::presets
