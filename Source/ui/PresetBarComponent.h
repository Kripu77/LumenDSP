#pragma once

#include <JuceHeader.h>
#include "ui/DesignTokens.h"

namespace lumen::ui
{

class PresetBarComponent : public juce::Component
{
public:
    PresetBarComponent();

    void resized() override;
    void setPresetNames(const juce::StringArray& presetNames, const juce::String& selectedName);
    juce::String getSelectedPresetName() const;
    juce::String getEditorPresetName() const;

    std::function<void()> onSaveRequested;
    std::function<void()> onLoadRequested;
    std::function<void()> onDeleteRequested;
    std::function<void(const juce::String&)> onPresetSelected;

private:
    juce::Label titleLabel;
    juce::ComboBox presetCombo;
    juce::TextEditor nameEditor;
    juce::TextButton saveButton{"Save"};
    juce::TextButton loadButton{"Load"};
    juce::TextButton deleteButton{"Delete"};
};

} // namespace lumen::ui
