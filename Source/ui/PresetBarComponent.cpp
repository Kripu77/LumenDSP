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

    nameEditor.setFont(design::bodyFont());
    nameEditor.setColour(juce::TextEditor::backgroundColourId, juce::Colours::transparentBlack);
    nameEditor.setColour(juce::TextEditor::outlineColourId, juce::Colours::transparentBlack);
    nameEditor.setColour(juce::TextEditor::focusedOutlineColourId, juce::Colours::transparentBlack);
    nameEditor.setColour(juce::TextEditor::textColourId, design::textPrimary());
    nameEditor.setTextToShowWhenEmpty("Name", design::textMuted());
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
    graphics.setColour(design::bgElevated());
    graphics.fillRoundedRectangle(bounds, 18.0f);
    graphics.setColour(design::borderLight());
    graphics.drawRoundedRectangle(bounds, 18.0f, 1.0f);

    graphics.setColour(design::accent());
    graphics.fillEllipse(12.0f, bounds.getCentreY() - 4.0f, 8.0f, 8.0f);
}

void PresetBarComponent::resized()
{
    auto bounds = getLocalBounds().reduced(14, 5);
    bounds.removeFromLeft(14);

    deleteButton.setBounds(bounds.removeFromRight(44));
    bounds.removeFromRight(4);
    loadButton.setBounds(bounds.removeFromRight(52));
    bounds.removeFromRight(4);
    saveButton.setBounds(bounds.removeFromRight(52));
    bounds.removeFromRight(8);

    const int split = juce::jmax(120, bounds.getWidth() / 2);
    nameEditor.setBounds(bounds.removeFromRight(juce::jmin(140, split - 8)));
    bounds.removeFromRight(6);
    presetCombo.setBounds(bounds);
}

void PresetBarComponent::setPresetNames(const juce::StringArray& presetNames, const juce::String& selectedName)
{
    const auto previous = presetCombo.getText();
    presetCombo.clear(juce::dontSendNotification);
    int selectedId = 0;

    for (int index = 0; index < presetNames.size(); ++index)
    {
        const int itemId = index + 1;
        presetCombo.addItem(presetNames[index], itemId);
        if (presetNames[index] == selectedName
            || (selectedName.isEmpty() && presetNames[index] == previous))
            selectedId = itemId;
    }

    if (selectedId == 0 && presetNames.size() > 0)
        selectedId = 1;

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
