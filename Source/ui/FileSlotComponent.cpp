#include "ui/FileSlotComponent.h"

namespace lumen::ui
{

FileSlotComponent::FileSlotComponent(const juce::String& titleText, const juce::String& fileWildcard)
    : title(titleText)
    , wildcard(fileWildcard)
{
    browseButton.onClick = [this]() { openFileChooser(); };
    addAndMakeVisible(browseButton);
}

void FileSlotComponent::paint(juce::Graphics& graphics)
{
    auto bounds = getLocalBounds().toFloat().reduced(0.5f);
    const auto fill = dragActive ? design::accentSoft() : design::bgTertiary();
    const auto border = dragActive || busy ? design::accent() : design::borderLight();

    graphics.setColour(fill);
    graphics.fillRoundedRectangle(bounds, static_cast<float>(design::controlCornerRadiusPixels));
    graphics.setColour(border);
    graphics.drawRoundedRectangle(bounds, static_cast<float>(design::controlCornerRadiusPixels), 1.2f);

    auto content = getLocalBounds().reduced(design::spacingTwoUnitsPixels);
    auto titleArea = content.removeFromTop(design::spacingTwoUnitsPixels);
    auto statusArea = content.removeFromBottom(design::spacingTwoUnitsPixels);
    content.removeFromRight(100);
    auto fileNameArea = content;

    graphics.setColour(design::textSecondary());
    graphics.setFont(design::sectionFont());
    graphics.drawText(title, titleArea, juce::Justification::centredLeft);

    graphics.setColour(design::textPrimary());
    graphics.setFont(design::bodyFont());
    graphics.drawFittedText(displayedFileName, fileNameArea, juce::Justification::centredLeft, 1);

    if (statusText.isNotEmpty())
    {
        graphics.setColour(busy ? design::accent() : design::textMuted());
        graphics.setFont(design::microFont());
        graphics.drawText(statusText, statusArea, juce::Justification::centredLeft);
    }
}

void FileSlotComponent::resized()
{
    auto bounds = getLocalBounds().reduced(design::spacingTwoUnitsPixels);
    browseButton.setBounds(bounds.removeFromRight(96).withSizeKeepingCentre(96, 32));
}

void FileSlotComponent::mouseUp(const juce::MouseEvent& event)
{
    if (event.mouseWasClicked() && !browseButton.getBounds().contains(event.getPosition()))
        openFileChooser();
}

bool FileSlotComponent::isInterestedInFileDrag(const juce::StringArray& files)
{
    return !files.isEmpty() && matchesWildcard(juce::File(files[0]));
}

void FileSlotComponent::fileDragEnter(const juce::StringArray&, int, int)
{
    dragActive = true;
    repaint();
}

void FileSlotComponent::fileDragExit(const juce::StringArray&)
{
    dragActive = false;
    repaint();
}

void FileSlotComponent::filesDropped(const juce::StringArray& files, int, int)
{
    dragActive = false;
    repaint();

    if (files.isEmpty())
        return;

    const juce::File droppedFile(files[0]);
    if (!matchesWildcard(droppedFile))
        return;

    if (onFileChosen)
        onFileChosen(droppedFile);
}

void FileSlotComponent::setFileName(const juce::String& fileName)
{
    displayedFileName = fileName.isNotEmpty() ? fileName : "No file loaded";
    repaint();
}

void FileSlotComponent::setStatusText(const juce::String& status)
{
    statusText = status;
    repaint();
}

void FileSlotComponent::setBusy(bool isBusy)
{
    busy = isBusy;
    repaint();
}

void FileSlotComponent::setDarkTheme(bool shouldUseDarkTheme)
{
    darkTheme = shouldUseDarkTheme;
    repaint();
}

void FileSlotComponent::openFileChooser()
{
    fileChooser = std::make_unique<juce::FileChooser>("Select " + title, juce::File{}, wildcard);
    constexpr auto browserFlags =
        juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles;

    fileChooser->launchAsync(browserFlags, [this](const juce::FileChooser& chooser) {
        const auto result = chooser.getResult();
        if (result.existsAsFile() && onFileChosen)
            onFileChosen(result);
    });
}

bool FileSlotComponent::matchesWildcard(const juce::File& file) const
{
    return file.getFileExtension().isNotEmpty() && wildcard.containsIgnoreCase(file.getFileExtension());
}

} // namespace lumen::ui
