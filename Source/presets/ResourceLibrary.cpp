#include "presets/ResourceLibrary.h"
#include "presets/FactoryContentInstaller.h"
#include <algorithm>

namespace lumen::presets
{

ResourceLibrary::ResourceLibrary()
{
    ensureDirectories();
    loadFavorites();
    refresh();
}

juce::File ResourceLibrary::getLibraryRoot()
{
    return juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory)
        .getChildFile(applicationFolderName)
        .getChildFile(libraryFolderName);
}

juce::File ResourceLibrary::getUserModelsDirectory()
{
    return getLibraryRoot().getChildFile(modelsFolderName);
}

juce::File ResourceLibrary::getUserIrsDirectory()
{
    return getLibraryRoot().getChildFile(irsFolderName);
}

void ResourceLibrary::ensureDirectories() const
{
    getUserModelsDirectory().createDirectory();
    getUserIrsDirectory().createDirectory();
}

juce::File ResourceLibrary::favoritesFile() const
{
    return juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory)
        .getChildFile(applicationFolderName)
        .getChildFile(favoritesFileName);
}

void ResourceLibrary::loadFavorites()
{
    favoritePaths.clear();
    const auto file = favoritesFile();
    if (!file.existsAsFile())
        return;

    const auto parsed = juce::JSON::parse(file.loadFileAsString());
    if (!parsed.isArray())
        return;

    for (const auto& item : *parsed.getArray())
    {
        const auto path = item.toString();
        if (path.isNotEmpty())
            favoritePaths.addIfNotAlreadyThere(path);
    }
}

void ResourceLibrary::saveFavorites() const
{
    juce::Array<juce::var> items;
    for (const auto& path : favoritePaths)
        items.add(path);

    const auto root = favoritesFile().getParentDirectory();
    root.createDirectory();
    favoritesFile().replaceWithText(juce::JSON::toString(juce::var(items), true));
}

void ResourceLibrary::scanDirectory(
    const juce::File& directory,
    const juce::String& kind,
    const juce::String& source,
    const juce::String& wildcard,
    juce::Array<ResourceEntry>& destination) const
{
    if (!directory.isDirectory())
        return;

    for (const auto& entry : juce::RangedDirectoryIterator(directory, false, wildcard, juce::File::findFiles))
    {
        const auto file = entry.getFile();
        ResourceEntry resource;
        resource.name = file.getFileNameWithoutExtension();
        resource.path = file.getFullPathName();
        resource.kind = kind;
        resource.source = source;
        resource.favorite = favoritePaths.contains(resource.path);
        destination.add(std::move(resource));
    }
}

void ResourceLibrary::refresh()
{
    ensureDirectories();
    loadFavorites();

    models.clearQuick();
    irs.clearQuick();

    scanDirectory(
        FactoryContentInstaller::getUserModelsDirectory(),
        "nam",
        "factory",
        "*.nam",
        models);
    scanDirectory(getUserModelsDirectory(), "nam", "user", "*.nam", models);

    scanDirectory(
        FactoryContentInstaller::getUserIrsDirectory(),
        "ir",
        "factory",
        "*.wav",
        irs);
    scanDirectory(getUserIrsDirectory(), "ir", "user", "*.wav", irs);

    auto byFavoriteThenName = [](const ResourceEntry& a, const ResourceEntry& b) {
        if (a.favorite != b.favorite)
            return a.favorite && !b.favorite;
        return a.name.compareIgnoreCase(b.name) < 0;
    };

    std::sort(models.begin(), models.end(), byFavoriteThenName);
    std::sort(irs.begin(), irs.end(), byFavoriteThenName);
}

juce::Array<ResourceEntry> ResourceLibrary::getModels() const
{
    return models;
}

juce::Array<ResourceEntry> ResourceLibrary::getIrs() const
{
    return irs;
}

bool ResourceLibrary::toggleFavorite(const juce::String& path)
{
    if (path.isEmpty())
        return false;

    if (favoritePaths.contains(path))
        favoritePaths.removeString(path);
    else
        favoritePaths.add(path);

    saveFavorites();
    refresh();
    return true;
}

bool ResourceLibrary::importFile(
    const juce::File& sourceFile,
    const juce::String& kind,
    juce::String& errorOut,
    juce::File* destinationOut)
{
    if (!sourceFile.existsAsFile())
    {
        errorOut = "File does not exist.";
        return false;
    }

    ensureDirectories();

    juce::File destinationDirectory;
    if (kind == "nam")
    {
        if (!sourceFile.hasFileExtension(".nam"))
        {
            errorOut = "Expected a .nam model file.";
            return false;
        }
        destinationDirectory = getUserModelsDirectory();
    }
    else if (kind == "ir")
    {
        if (!sourceFile.hasFileExtension(".wav"))
        {
            errorOut = "Expected a .wav impulse response.";
            return false;
        }
        destinationDirectory = getUserIrsDirectory();
    }
    else
    {
        errorOut = "Unknown resource kind.";
        return false;
    }

    auto destination = destinationDirectory.getChildFile(sourceFile.getFileName());
    if (destination.existsAsFile())
    {
        const auto stem = sourceFile.getFileNameWithoutExtension();
        const auto ext = sourceFile.getFileExtension();
        int index = 2;
        while (destination.existsAsFile())
        {
            destination = destinationDirectory.getChildFile(stem + " " + juce::String(index) + ext);
            ++index;
        }
    }

    if (!sourceFile.copyFileTo(destination))
    {
        errorOut = "Could not copy file into the library.";
        return false;
    }

    if (destinationOut != nullptr)
        *destinationOut = destination;

    refresh();
    return true;
}

} // namespace lumen::presets
