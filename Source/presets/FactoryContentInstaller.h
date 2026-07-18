#pragma once

#include <JuceHeader.h>
#include "presets/PresetManager.h"

namespace lumen::presets
{

class FactoryContentInstaller
{
public:
    struct InstallResult
    {
        bool installedOrAlreadyPresent = false;
        bool performedFreshInstall = false;
        juce::String defaultPresetName;
        juce::File modelsDirectory;
        juce::File irsDirectory;
        juce::String statusMessage;
    };

    static juce::File getUserContentRoot();
    static juce::File getUserModelsDirectory();
    static juce::File getUserIrsDirectory();
    static juce::File getMarkerFile();

    static juce::File findBundledFactoryRoot();
    static InstallResult installIfNeeded(PresetManager& presetManager);
    static bool writeFactoryPresets(PresetManager& presetManager,
                                    const juce::File& modelsDirectory,
                                    const juce::File& irsDirectory);

private:
    static constexpr const char* applicationFolderName = "LumenDSP";
    static constexpr const char* contentFolderName = "FactoryContent";
    static constexpr const char* modelsFolderName = "Models";
    static constexpr const char* irsFolderName = "IRs";
    static constexpr const char* installMarkerFileName = ".factory_content_v1";
    static constexpr const char* defaultPresetName = "01 Glass Clean";

    static bool copyDirectoryRecursive(const juce::File& source, const juce::File& destination);
};

} // namespace lumen::presets
