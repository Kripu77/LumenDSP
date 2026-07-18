#include "ui/PresetBarComponent.h"

namespace lumen::ui
{

PresetBarComponent::PresetBarComponent()
{
    presetCombo.setTextWhenNothingSelected("Select preset");
    presetCombo.onChange = [this]() {
        if (onPresetSelected)
            onPresetSelected(presetCombo.getText());
    };
    addAndMakeVisible(presetCombo);

    nameEditor.setTextToShowWhenEmpty("Preset name", design::chromeTextMuted());
    nameEditor.setFont(design::bodyFont());
    nameEditor.setColour(juce::TextEditor::backgroundColourId, design::chromeSurfaceRaised());
    nameEditor.setColour(juce::TextEditor::outlineColourId, design::chromeBorder());
    nameEditor.setColour(juce::TextEditor::textColourId, design::chromeTextPrimary());
    addAndMakeVisible(nameEditor);

    saveButton.onClick = [this]() {
        if (onSaveRequested)
            onSaveRequested();
    };
    loadButton.onClick = [this]() {
        if (onLoadRequested)
            onLoadRequested();
    };
    deleteButton.onClick = [this]() {
        if (onDeleteRequested)
            onDeleteRequested();
    };

    addAndMakeVisible(saveButton);
    addAndMakeVisible(loadButton);
    addAndMakeVisible(deleteButton);
}

void PresetBarComponent::paint(juce::Graphics& graphics)
{
    auto bounds = getLocalBounds().toFloat().reduced(0.5f);
    graphics.setColour(design::chromeSurfaceRaised());
    graphics.fillRoundedRectangle(bounds, 18.0f);
    graphics.setColour(design::chromeBorder());
    graphics.drawRoundedRectangle(bounds, 18.0f, 1.0f);
}

void PresetBarComponent::resized()
{
    auto bounds = getLocalBounds().reduced(10, 6);
    deleteButton.setBounds(bounds.removeFromRight(48));
    bounds.removeFromRight(4);
    loadButton.setBounds(bounds.removeFromRight(56));
    bounds.removeFromRight(4);
    saveButton.setBounds(bounds.removeFromRight(56));
    bounds.removeFromRight(8);
    nameEditor.setBounds(bounds.removeFromRight(140));
    bounds.removeFromRight(8);
    presetCombo.setBounds(bounds);
}

void PresetBarComponent::setPresetNames(const juce::StringArray& presetNames, const juce::String& selectedName)
{
    presetCombo.clear(juce::dontSendNotification);
    int selectedId = 0;

    for (int index = 0; index < presetNames.size(); ++index)
    {
        const int itemId = index + 1;
        presetCombo.addItem(presetNames[index], itemId);
        if (presetNames[index] == selectedName)
            selectedId = itemId;
    }

    if (selectedId > 0)
        presetCombo.setSelectedId(selectedId, juce::dontSendNotification);
}

juce::String PresetBarComponent::getSelectedPresetName() const
{
    return presetCombo.getText();
}

juce::String PresetBarComponent::getEditorPresetName() const
{
    const auto typedName = nameEditor.getText().trim();
    if (typedName.isNotEmpty())
        return typedName;
    return getSelectedPresetName();
}

} // namespace lumen::ui
