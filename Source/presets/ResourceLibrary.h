#pragma once

#include <JuceHeader.h>

namespace lumen::presets
{

struct ResourceEntry
{
    juce::String name;
    juce::String path;
    juce::String kind;
    juce::String source;
    bool favorite = false;
};

class ResourceLibrary
{
public:
    ResourceLibrary();

    void refresh();
    juce::Array<ResourceEntry> getModels() const;
    juce::Array<ResourceEntry> getIrs() const;
    bool toggleFavorite(const juce::String& path);
    bool importFile(
        const juce::File& sourceFile,
        const juce::String& kind,
        juce::String& errorOut,
        juce::File* destinationOut = nullptr);

    static juce::File getLibraryRoot();
    static juce::File getUserModelsDirectory();
    static juce::File getUserIrsDirectory();

private:
    static constexpr const char* applicationFolderName = "LumenDSP";
    static constexpr const char* libraryFolderName = "Library";
    static constexpr const char* modelsFolderName = "Models";
    static constexpr const char* irsFolderName = "IRs";
    static constexpr const char* favoritesFileName = "library-favorites.json";

    void ensureDirectories() const;
    void loadFavorites();
    void saveFavorites() const;
    void scanDirectory(
        const juce::File& directory,
        const juce::String& kind,
        const juce::String& source,
        const juce::String& wildcard,
        juce::Array<ResourceEntry>& destination) const;

    juce::File favoritesFile() const;

    juce::Array<ResourceEntry> models;
    juce::Array<ResourceEntry> irs;
    juce::StringArray favoritePaths;
};

} // namespace lumen::presets
