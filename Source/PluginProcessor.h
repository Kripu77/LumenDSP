#pragma once

#include <JuceHeader.h>
#include "audio/AudioPipeline.h"
#include "parameters/ParameterIds.h"
#include "presets/FactoryContentInstaller.h"
#include "presets/PresetManager.h"
#include "presets/ResourceLibrary.h"

class LumenDSPAudioProcessor : public juce::AudioProcessor
{
public:
    LumenDSPAudioProcessor();
    ~LumenDSPAudioProcessor() override;

    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    bool isBusesLayoutSupported(const BusesLayout& layouts) const override;
    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    const juce::String getName() const override;
    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram(int index) override;
    const juce::String getProgramName(int index) override;
    void changeProgramName(int index, const juce::String& newName) override;

    void getStateInformation(juce::MemoryBlock& destData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;

    juce::AudioProcessorValueTreeState& getValueTreeState() noexcept;
    lumen::audio::AudioPipeline& getAudioPipeline() noexcept;
    lumen::presets::PresetManager& getPresetManager() noexcept;
    lumen::presets::ResourceLibrary& getResourceLibrary() noexcept;
    const lumen::presets::ResourceLibrary& getResourceLibrary() const noexcept;

    void requestNamLoad(const juce::File& modelFile);
    void requestIrLoad(const juce::File& irFile);
    bool applyPreset(const juce::String& presetName);
    bool storePreset(
        const juce::String& presetName,
        const juce::String& category = {},
        const juce::StringArray& tags = {});
    void ensureFactoryContentReady();
    juce::String getFactoryStatusMessage() const;
    juce::String getDefaultPresetName() const;

private:
    static constexpr int defaultInputChannelCount = 2;
    static constexpr int defaultOutputChannelCount = 2;
    static constexpr double defaultTailLengthSeconds = 0.0;
    static constexpr const char* stateRootType = "LumenDSPState";
    static constexpr const char* namPathProperty = "namModelPath";
    static constexpr const char* irPathProperty = "irPath";

    lumen::audio::PipelineControlState readControlState() const;
    void restoreAttachedFilesFromState(const juce::ValueTree& stateTree);

    void bootstrapFactoryExperience();

    juce::AudioProcessorValueTreeState apvts;
    lumen::audio::AudioPipeline audioPipeline;
    lumen::presets::PresetManager presetManager;
    lumen::presets::ResourceLibrary resourceLibrary;
    juce::ThreadPool backgroundLoadPool{1};
    juce::String factoryStatusMessage;
    juce::String defaultPresetName{"01 Glass Clean"};
    bool factoryBootstrapComplete = false;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(LumenDSPAudioProcessor)
};
