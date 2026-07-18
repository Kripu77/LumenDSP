#pragma once

#include <JuceHeader.h>
#include "ui/DesignTokens.h"

namespace lumen::ui
{

class FileSlotComponent : public juce::Component,
                          public juce::FileDragAndDropTarget
{
public:
    FileSlotComponent(const juce::String& titleText, const juce::String& fileWildcard);

    void paint(juce::Graphics& graphics) override;
    void resized() override;
    void mouseUp(const juce::MouseEvent& event) override;

    bool isInterestedInFileDrag(const juce::StringArray& files) override;
    void fileDragEnter(const juce::StringArray& files, int x, int y) override;
    void fileDragExit(const juce::StringArray& files) override;
    void filesDropped(const juce::StringArray& files, int x, int y) override;

    void setFileName(const juce::String& fileName);
    void setStatusText(const juce::String& status);
    void setBusy(bool isBusy);
    void setDarkTheme(bool shouldUseDarkTheme);

    std::function<void(const juce::File&)> onFileChosen;

private:
    void openFileChooser();
    bool matchesWildcard(const juce::File& file) const;

    juce::String title;
    juce::String wildcard;
    juce::String displayedFileName{"No file loaded"};
    juce::String statusText;
    bool dragActive = false;
    bool busy = false;
    bool darkTheme = false;
    juce::TextButton browseButton{"BROWSE"};
    std::unique_ptr<juce::FileChooser> fileChooser;
};

} // namespace lumen::ui
