#include "presets/PresetManager.h"
#include <algorithm>
#include <cstdint>

namespace lumen::presets
{

PresetManager::PresetManager(juce::AudioProcessorValueTreeState& valueTreeState)
    : apvts(valueTreeState)
{
    getPresetDirectory().createDirectory();
    loadFavorites();
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

juce::File PresetManager::favoritesFile() const
{
    return juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory)
        .getChildFile(applicationFolderName)
        .getChildFile(favoritesFileName);
}

void PresetManager::loadFavorites()
{
    favoriteNames.clear();
    const auto file = favoritesFile();
    if (!file.existsAsFile())
        return;

    const auto parsed = juce::JSON::parse(file.loadFileAsString());
    if (!parsed.isArray())
        return;

    for (const auto& item : *parsed.getArray())
    {
        const auto name = item.toString();
        if (name.isNotEmpty())
            favoriteNames.addIfNotAlreadyThere(name);
    }
}

void PresetManager::saveFavorites() const
{
    juce::Array<juce::var> items;
    for (const auto& name : favoriteNames)
        items.add(name);

    favoritesFile().getParentDirectory().createDirectory();
    favoritesFile().replaceWithText(juce::JSON::toString(juce::var(items), true));
}

juce::String PresetManager::tagsToString(const juce::StringArray& tags)
{
    juce::StringArray cleaned;
    for (const auto& tag : tags)
    {
        const auto trimmed = tag.trim();
        if (trimmed.isNotEmpty())
            cleaned.addIfNotAlreadyThere(trimmed);
    }
    return cleaned.joinIntoString(", ");
}

juce::StringArray PresetManager::tagsFromString(const juce::String& tags)
{
    juce::StringArray parsed;
    parsed.addTokens(tags, ",", "\"");
    parsed.trim();
    parsed.removeEmptyStrings();
    return parsed;
}

juce::String PresetManager::inferCategory(const juce::String& presetName)
{
    const auto lower = presetName.toLowerCase();
    if (lower.contains("clean") || lower.contains("glass") || lower.contains("airy"))
        return "Clean";
    if (lower.contains("lead") || lower.contains("sustain"))
        return "Lead";
    if (lower.contains("crunch") || lower.contains("drive") || lower.contains("rock"))
        return "Crunch";
    if (lower.contains("ambient") || lower.contains("verb") || lower.contains("space"))
        return "Ambient";
    return "User";
}

juce::StringArray PresetManager::getPresetNames() const
{
    juce::StringArray presetNames;
    for (const auto& summary : getPresetSummaries())
        presetNames.add(summary.name);
    return presetNames;
}

juce::Array<PresetSummary> PresetManager::getPresetSummaries() const
{
    juce::Array<PresetSummary> summaries;
    const auto presetFiles = getPresetDirectory().findChildFiles(
        juce::File::findFiles,
        false,
        juce::String("*") + presetFileExtension);

    for (const auto& presetFile : presetFiles)
    {
        PresetSummary summary;
        summary.name = presetFile.getFileNameWithoutExtension();
        summary.category = inferCategory(summary.name);
        summary.favorite = favoriteNames.contains(summary.name);

        if (auto xml = juce::XmlDocument::parse(presetFile))
        {
            auto tree = juce::ValueTree::fromXml(*xml);
            if (tree.hasType(rootXmlTag))
            {
                const auto name = tree.getProperty(nameAttribute).toString();
                if (name.isNotEmpty())
                    summary.name = name;

                const auto category = tree.getProperty(categoryAttribute).toString().trim();
                if (category.isNotEmpty())
                    summary.category = category;
                else
                    summary.category = inferCategory(summary.name);

                summary.tags = tagsFromString(tree.getProperty(tagsAttribute).toString());
                summary.favorite = favoriteNames.contains(summary.name);
            }
        }

        summaries.add(std::move(summary));
    }

    std::sort(summaries.begin(), summaries.end(), [](const PresetSummary& a, const PresetSummary& b) {
        if (a.favorite != b.favorite)
            return a.favorite && !b.favorite;
        const int categoryCompare = a.category.compareIgnoreCase(b.category);
        if (categoryCompare != 0)
            return categoryCompare < 0;
        return a.name.compareIgnoreCase(b.name) < 0;
    });

    return summaries;
}

juce::StringArray PresetManager::getCategories() const
{
    juce::StringArray categories;
    for (const auto& summary : getPresetSummaries())
        if (summary.category.isNotEmpty())
            categories.addIfNotAlreadyThere(summary.category);
    categories.sort(true);
    return categories;
}

bool PresetManager::savePreset(
    const juce::String& presetName,
    const juce::File& namModelFile,
    const juce::File& irFile,
    const juce::String& category,
    const juce::StringArray& tags)
{
    const auto sanitizedName = sanitizePresetName(presetName);
    if (sanitizedName.isEmpty())
        return false;

    juce::ValueTree presetTree(rootXmlTag);
    presetTree.setProperty(nameAttribute, sanitizedName, nullptr);
    presetTree.setProperty(
        categoryAttribute,
        category.trim().isNotEmpty() ? category.trim() : inferCategory(sanitizedName),
        nullptr);
    presetTree.setProperty(tagsAttribute, tagsToString(tags), nullptr);
    presetTree.setProperty(namPathAttribute, namModelFile.getFullPathName(), nullptr);
    presetTree.setProperty(irPathAttribute, irFile.getFullPathName(), nullptr);

    const auto parameterState = apvts.copyState();
    juce::ValueTree parametersTree(parametersChildTag);
    parametersTree.copyPropertiesAndChildrenFrom(parameterState, nullptr);
    presetTree.addChild(parametersTree, -1, nullptr);

    const auto presetFile = resolvePresetFile(sanitizedName);
    std::unique_ptr<juce::XmlElement> xml(presetTree.createXml());
    if (xml == nullptr || !xml->writeTo(presetFile))
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
    presetDataOut.category = presetTree.getProperty(categoryAttribute).toString().trim();
    if (presetDataOut.category.isEmpty())
        presetDataOut.category = inferCategory(presetDataOut.name);
    presetDataOut.tags = tagsFromString(presetTree.getProperty(tagsAttribute).toString());
    presetDataOut.namModelPath = presetTree.getProperty(namPathAttribute).toString();
    presetDataOut.irPath = presetTree.getProperty(irPathAttribute).toString();
    presetDataOut.favorite = favoriteNames.contains(presetDataOut.name);

    auto parametersTree = presetTree.getChildWithName(parametersChildTag);
    if (!parametersTree.isValid())
        return false;

    presetDataOut.parameterState = parametersTree;
    currentPresetName = presetDataOut.name;
    return true;
}

bool PresetManager::deletePreset(const juce::String& presetName)
{
    const auto sanitized = sanitizePresetName(presetName);
    const auto presetFile = resolvePresetFile(sanitized);
    if (!presetFile.existsAsFile())
        return false;

    const bool removed = presetFile.deleteFile();
    if (removed)
    {
        favoriteNames.removeString(sanitized);
        saveFavorites();
        if (currentPresetName == sanitized)
            currentPresetName.clear();
    }
    return removed;
}

bool PresetManager::renamePreset(const juce::String& oldName, const juce::String& newName)
{
    PresetData data;
    if (!loadPreset(oldName, data))
        return false;

    const auto sanitizedNew = sanitizePresetName(newName);
    if (sanitizedNew.isEmpty() || sanitizedNew == sanitizePresetName(oldName))
        return false;

    if (resolvePresetFile(sanitizedNew).existsAsFile())
        return false;

    data.name = sanitizedNew;
    juce::ValueTree presetTree(rootXmlTag);
    presetTree.setProperty(nameAttribute, data.name, nullptr);
    presetTree.setProperty(categoryAttribute, data.category, nullptr);
    presetTree.setProperty(tagsAttribute, tagsToString(data.tags), nullptr);
    presetTree.setProperty(namPathAttribute, data.namModelPath, nullptr);
    presetTree.setProperty(irPathAttribute, data.irPath, nullptr);

    juce::ValueTree parametersTree(parametersChildTag);
    parametersTree.copyPropertiesAndChildrenFrom(data.parameterState, nullptr);
    presetTree.addChild(parametersTree, -1, nullptr);

    std::unique_ptr<juce::XmlElement> xml(presetTree.createXml());
    if (xml == nullptr || !xml->writeTo(resolvePresetFile(sanitizedNew)))
        return false;

    resolvePresetFile(oldName).deleteFile();

    if (favoriteNames.contains(sanitizePresetName(oldName)))
    {
        favoriteNames.removeString(sanitizePresetName(oldName));
        favoriteNames.addIfNotAlreadyThere(sanitizedNew);
        saveFavorites();
    }

    if (currentPresetName == sanitizePresetName(oldName))
        currentPresetName = sanitizedNew;

    return true;
}

bool PresetManager::toggleFavorite(const juce::String& presetName)
{
    const auto sanitized = sanitizePresetName(presetName);
    if (sanitized.isEmpty() || !resolvePresetFile(sanitized).existsAsFile())
        return false;

    if (favoriteNames.contains(sanitized))
        favoriteNames.removeString(sanitized);
    else
        favoriteNames.add(sanitized);

    saveFavorites();
    return true;
}

bool PresetManager::exportPreset(const juce::String& presetName, const juce::File& destinationFile) const
{
    const auto source = resolvePresetFile(sanitizePresetName(presetName));
    if (!source.existsAsFile() || destinationFile.getFullPathName().isEmpty())
        return false;

    if (destinationFile.existsAsFile())
        destinationFile.deleteFile();

    return source.copyFileTo(destinationFile);
}

bool PresetManager::embedFileResource(
    juce::ValueTree& resourcesTree,
    const juce::String& kind,
    const juce::File& file,
    juce::String& errorOut)
{
    if (!file.existsAsFile())
        return true;

    // Preset paths are untrusted text; only ever embed real model/IR files.
    if (!file.hasFileExtension(kind == "ir" ? ".wav" : ".nam"))
    {
        errorOut = "Refusing to embed unexpected file type: " + file.getFileName();
        return false;
    }

    juce::MemoryBlock data;
    if (!file.loadFileAsData(data) || data.isEmpty())
    {
        errorOut = "Could not read resource: " + file.getFileName();
        return false;
    }

    // Keep packs practical for typical NAM + IR sizes.
    constexpr size_t maxBytes = 40ull * 1024ull * 1024ull;
    if (data.getSize() > maxBytes)
    {
        errorOut = "Resource too large to embed: " + file.getFileName();
        return false;
    }

    juce::ValueTree resource(resourceTag);
    // FNV-1a 64-bit — good enough for pack identity without cryptography module.
    std::uint64_t hash = 14695981039346656037ull;
    const auto* bytes = static_cast<const std::uint8_t*>(data.getData());
    for (size_t i = 0; i < data.getSize(); ++i)
    {
        hash ^= bytes[i];
        hash *= 1099511628211ull;
    }

    resource.setProperty("kind", kind, nullptr);
    resource.setProperty("name", file.getFileName(), nullptr);
    resource.setProperty("hash", juce::String::toHexString(static_cast<juce::int64>(hash)), nullptr);
    resource.setProperty("encoding", "base64", nullptr);
    resource.setProperty("data", juce::Base64::toBase64(data.getData(), data.getSize()), nullptr);
    resourcesTree.appendChild(resource, nullptr);
    return true;
}

bool PresetManager::exportPresetPack(
    const juce::String& presetName,
    const juce::File& destinationFile,
    juce::String& errorOut) const
{
    const auto presetFile = resolvePresetFile(sanitizePresetName(presetName));
    if (!presetFile.existsAsFile())
    {
        errorOut = "Preset not found.";
        return false;
    }

    auto sourceXml = juce::XmlDocument::parse(presetFile);
    if (sourceXml == nullptr)
    {
        errorOut = "Could not parse preset.";
        return false;
    }

    auto packTree = juce::ValueTree::fromXml(*sourceXml);
    if (!packTree.hasType(rootXmlTag))
    {
        errorOut = "Not a LumenDSP preset file.";
        return false;
    }

    packTree.setProperty("packVersion", 1, nullptr);

    // Remove any stale embedded payload before re-embedding current files.
    auto existingEmbedded = packTree.getChildWithName(embeddedResourcesTag);
    if (existingEmbedded.isValid())
        packTree.removeChild(existingEmbedded, nullptr);

    juce::ValueTree resourcesTree(embeddedResourcesTag);
    const juce::File namFile(packTree.getProperty(namPathAttribute).toString());
    const juce::File irFile(packTree.getProperty(irPathAttribute).toString());
    if (!embedFileResource(resourcesTree, "nam", namFile, errorOut))
        return false;
    if (!embedFileResource(resourcesTree, "ir", irFile, errorOut))
        return false;
    packTree.addChild(resourcesTree, -1, nullptr);

    auto dest = destinationFile;
    if (!dest.hasFileExtension(packFileExtension) && !dest.hasFileExtension(presetFileExtension))
        dest = dest.withFileExtension(packFileExtension);

    if (dest.existsAsFile())
        dest.deleteFile();

    std::unique_ptr<juce::XmlElement> outXml(packTree.createXml());
    if (outXml == nullptr || !outXml->writeTo(dest))
    {
        errorOut = "Could not write preset pack.";
        return false;
    }

    return true;
}

bool PresetManager::extractEmbeddedResources(
    juce::ValueTree& presetTree,
    juce::String& errorOut)
{
    auto resourcesTree = presetTree.getChildWithName(embeddedResourcesTag);
    if (!resourcesTree.isValid())
        return true;

    const auto importRoot = juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory)
                                .getChildFile(applicationFolderName)
                                .getChildFile("Library")
                                .getChildFile("Imported");
    const auto modelsDir = importRoot.getChildFile("Models");
    const auto irsDir = importRoot.getChildFile("IRs");
    modelsDir.createDirectory();
    irsDir.createDirectory();

    for (int i = 0; i < resourcesTree.getNumChildren(); ++i)
    {
        auto resource = resourcesTree.getChild(i);
        if (!resource.hasType(resourceTag))
            continue;

        const auto kind = resource.getProperty("kind").toString();
        // Packs come from other people: reduce the name to a bare, legal file name of the right type.
        const auto name = juce::File::createLegalFileName(resource.getProperty("name").toString()
                                                               .fromLastOccurrenceOf("/", false, false)
                                                               .fromLastOccurrenceOf("\\", false, false));
        const auto dataB64 = resource.getProperty("data").toString();
        if ((kind != "nam" && kind != "ir") || name.isEmpty() || name.startsWith(".") || dataB64.isEmpty())
            continue;
        if (!name.endsWithIgnoreCase(kind == "ir" ? ".wav" : ".nam"))
            continue;

        juce::MemoryOutputStream decoded;
        if (!juce::Base64::convertFromBase64(decoded, dataB64) || decoded.getDataSize() == 0)
        {
            errorOut = "Could not decode embedded resource: " + name;
            return false;
        }

        const auto hash = resource.getProperty("hash").toString().toLowerCase().retainCharacters("0123456789abcdef");
        auto targetDir = kind == "ir" ? irsDir : modelsDir;
        juce::String fileName = name;
        if (hash.isNotEmpty())
            fileName = hash.substring(0, 12) + "_" + name;

        auto target = targetDir.getChildFile(fileName);
        if (!target.isAChildOf(targetDir))
        {
            errorOut = "Rejected embedded resource with an unsafe name.";
            return false;
        }
        if (!target.existsAsFile())
        {
            if (!target.replaceWithData(decoded.getData(), decoded.getDataSize()))
            {
                errorOut = "Could not write imported resource: " + name;
                return false;
            }
        }

        if (kind == "nam")
            presetTree.setProperty(namPathAttribute, target.getFullPathName(), nullptr);
        else if (kind == "ir")
            presetTree.setProperty(irPathAttribute, target.getFullPathName(), nullptr);
    }

    // Drop bulky payloads after extraction so local preset files stay small.
    presetTree.removeChild(resourcesTree, nullptr);
    return true;
}

bool PresetManager::importPreset(
    const juce::File& sourceFile,
    juce::String& errorOut,
    juce::String* importedNameOut)
{
    if (!sourceFile.existsAsFile())
    {
        errorOut = "Preset file does not exist.";
        return false;
    }

    auto xml = juce::XmlDocument::parse(sourceFile);
    if (xml == nullptr)
    {
        errorOut = "Could not parse preset file.";
        return false;
    }

    auto presetTree = juce::ValueTree::fromXml(*xml);
    if (!presetTree.hasType(rootXmlTag))
    {
        errorOut = "Not a LumenDSP preset file.";
        return false;
    }

    if (!extractEmbeddedResources(presetTree, errorOut))
        return false;

    auto name = sanitizePresetName(presetTree.getProperty(nameAttribute).toString());
    if (name.isEmpty())
        name = sanitizePresetName(sourceFile.getFileNameWithoutExtension());
    if (name.isEmpty())
    {
        errorOut = "Preset has no valid name.";
        return false;
    }

    auto destination = resolvePresetFile(name);
    if (destination.existsAsFile())
    {
        int index = 2;
        auto candidate = name;
        while (resolvePresetFile(candidate).existsAsFile())
        {
            candidate = name + " " + juce::String(index);
            ++index;
        }
        name = candidate;
        presetTree.setProperty(nameAttribute, name, nullptr);
        destination = resolvePresetFile(name);
    }

    if (presetTree.getProperty(categoryAttribute).toString().trim().isEmpty())
        presetTree.setProperty(categoryAttribute, inferCategory(name), nullptr);

    std::unique_ptr<juce::XmlElement> outXml(presetTree.createXml());
    if (outXml == nullptr || !outXml->writeTo(destination))
    {
        errorOut = "Could not write imported preset.";
        return false;
    }

    if (importedNameOut != nullptr)
        *importedNameOut = name;

    return true;
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
    return getPresetDirectory().getChildFile(sanitizePresetName(presetName) + presetFileExtension);
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
