#include "ui/PresetBarComponent.h"

namespace lumen::ui
{

PresetBarComponent::PresetBarComponent()
{
    titleLabel.setText("Presets", juce::dontSendNotification);
    titleLabel.setFont(design::sectionFont());
    titleLabel.setColour(juce::Label::textColourId, design::textSecondary());
    addAndMakeVisible(titleLabel);

    presetCombo.setTextWhenNothingSelected("Select preset");
    presetCombo.onChange = [this]() {
        if (onPresetSelected)
            onPresetSelected(presetCombo.getText());
    };
    addAndMakeVisible(presetCombo);

    nameEditor.setTextToShowWhenEmpty("Preset name", design::textMuted());
    nameEditor.setFont(design::bodyFont());
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

void PresetBarComponent::resized()
{
    auto bounds = getLocalBounds();
    titleLabel.setBounds(bounds.removeFromLeft(72));
    bounds.removeFromLeft(design::spacingUnitPixels);

    deleteButton.setBounds(bounds.removeFromRight(80));
    bounds.removeFromRight(design::spacingHalfUnitPixels);
    loadButton.setBounds(bounds.removeFromRight(72));
    bounds.removeFromRight(design::spacingHalfUnitPixels);
    saveButton.setBounds(bounds.removeFromRight(72));
    bounds.removeFromRight(design::spacingUnitPixels);

    nameEditor.setBounds(bounds.removeFromRight(160));
    bounds.removeFromRight(design::spacingUnitPixels);
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
