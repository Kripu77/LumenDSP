#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include <optional>

class LumenWebBrowser : public juce::WebBrowserComponent
{
public:
    using juce::WebBrowserComponent::WebBrowserComponent;
    bool pageAboutToLoad(const juce::String& newURL) override;
};

class LumenDSPAudioProcessorEditor : public juce::AudioProcessorEditor,
                                     private juce::Timer
{
public:
    explicit LumenDSPAudioProcessorEditor(LumenDSPAudioProcessor& audioProcessorToEdit);
    ~LumenDSPAudioProcessorEditor() override;

    void paint(juce::Graphics& graphics) override;
    void resized() override;

    void handleWebMessage(const juce::String& message);
    void pushStateToWeb();
    void pushMetersToWeb();

private:
    void timerCallback() override;
    void emitToWeb(const juce::var& payload);
    juce::var buildStateObject() const;
    juce::File getWebResourceRoot() const;
    std::optional<juce::WebBrowserComponent::Resource> getResource(const juce::String& url);
    juce::String getMimeForExtension(const juce::String& extension) const;
    void handleSetParameter(const juce::String& parameterId, float value);
    void handleBrowseNam();
    void handleBrowseIr();
    void handleOpenAudioSettings();
    float readParameter(const juce::String& parameterId) const;

    LumenDSPAudioProcessor& audioProcessor;
    juce::File webResourceRoot;
    LumenWebBrowser webView;
    std::unique_ptr<juce::FileChooser> fileChooser;
    bool webReady = false;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(LumenDSPAudioProcessorEditor)
};
