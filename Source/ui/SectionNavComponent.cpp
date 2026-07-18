#include "ui/SectionNavComponent.h"

namespace lumen::ui
{

SectionNavComponent::SectionNavComponent()
{
    items[static_cast<size_t>(EditorSection::input)] = {EditorSection::input, "INPUT", {}};
    items[static_cast<size_t>(EditorSection::amp)] = {EditorSection::amp, "AMP", {}};
    items[static_cast<size_t>(EditorSection::eq)] = {EditorSection::eq, "EQ", {}};
    items[static_cast<size_t>(EditorSection::cab)] = {EditorSection::cab, "CAB", {}};
}

void SectionNavComponent::paint(juce::Graphics& graphics)
{
    auto panel = getLocalBounds().toFloat().reduced(0.5f);
    design::drawChromePanel(graphics, panel, static_cast<float>(design::panelCornerRadiusPixels) * 0.75f);

    for (const auto& item : items)
        drawItem(graphics, item, item.section == activeSection);
}

void SectionNavComponent::resized()
{
    layoutItems();
}

void SectionNavComponent::mouseUp(const juce::MouseEvent& event)
{
    if (!event.mouseWasClicked())
        return;

    for (const auto& item : items)
    {
        if (item.bounds.contains(event.position))
        {
            setActiveSection(item.section);
            if (onSectionChanged)
                onSectionChanged(item.section);
            break;
        }
    }
}

void SectionNavComponent::setActiveSection(EditorSection section)
{
    activeSection = section;
    repaint();
}

EditorSection SectionNavComponent::getActiveSection() const noexcept
{
    return activeSection;
}

void SectionNavComponent::layoutItems()
{
    auto content = getLocalBounds().reduced(design::spacingTwoUnitsPixels, design::spacingUnitPixels).toFloat();
    const float itemWidth = content.getWidth() / static_cast<float>(items.size());

    for (size_t index = 0; index < items.size(); ++index)
    {
        items[index].bounds = juce::Rectangle<float>(
            content.getX() + itemWidth * static_cast<float>(index),
            content.getY(),
            itemWidth,
            content.getHeight());
    }
}

void SectionNavComponent::drawItem(juce::Graphics& graphics, const NavItem& item, bool isActive) const
{
    auto bounds = item.bounds.reduced(6.0f, 4.0f);
    if (isActive)
    {
        graphics.setColour(design::accentSoft());
        graphics.fillRoundedRectangle(bounds, 10.0f);
        graphics.setColour(design::accent());
        graphics.fillRoundedRectangle(
            bounds.removeFromBottom(design::sectionActiveIndicatorHeightPixels).withSizeKeepingCentre(28.0f, design::sectionActiveIndicatorHeightPixels),
            1.5f);
        bounds = item.bounds.reduced(6.0f, 4.0f).withTrimmedBottom(8.0f);
    }

    auto iconBounds = bounds.removeFromTop(bounds.getHeight() * 0.58f).withSizeKeepingCentre(28.0f, 22.0f);
    drawGlyph(graphics, item.section, iconBounds, isActive);

    graphics.setColour(isActive ? design::chromeTextPrimary() : design::chromeTextMuted());
    graphics.setFont(design::microFont());
    graphics.drawText(item.title, bounds, juce::Justification::centred);
}

void SectionNavComponent::drawGlyph(
    juce::Graphics& graphics,
    EditorSection section,
    juce::Rectangle<float> iconBounds,
    bool isActive) const
{
    graphics.setColour(isActive ? design::accent() : design::chromeTextSecondary());

    switch (section)
    {
        case EditorSection::input:
        {
            juce::Path wave;
            const float midY = iconBounds.getCentreY();
            wave.startNewSubPath(iconBounds.getX(), midY);
            wave.quadraticTo(iconBounds.getX() + iconBounds.getWidth() * 0.25f, midY - 8.0f, iconBounds.getCentreX(), midY);
            wave.quadraticTo(iconBounds.getX() + iconBounds.getWidth() * 0.75f, midY + 8.0f, iconBounds.getRight(), midY);
            graphics.strokePath(wave, juce::PathStrokeType(2.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
            break;
        }
        case EditorSection::amp:
        {
            graphics.drawRoundedRectangle(iconBounds.reduced(1.0f), 3.0f, 1.8f);
            for (int lineIndex = 0; lineIndex < 5; ++lineIndex)
            {
                const float x = iconBounds.getX() + 4.0f + static_cast<float>(lineIndex) * (iconBounds.getWidth() - 8.0f) / 4.0f;
                graphics.drawLine(x, iconBounds.getY() + 4.0f, x, iconBounds.getBottom() - 4.0f, 1.2f);
            }
            break;
        }
        case EditorSection::eq:
        {
            const float barWidth = 3.0f;
            const float heights[5] = {0.35f, 0.7f, 0.45f, 0.85f, 0.55f};
            for (int barIndex = 0; barIndex < 5; ++barIndex)
            {
                const float x = iconBounds.getX() + static_cast<float>(barIndex) * (iconBounds.getWidth() / 5.0f) + 2.0f;
                const float h = iconBounds.getHeight() * heights[barIndex];
                graphics.fillRoundedRectangle(x, iconBounds.getBottom() - h, barWidth, h, 1.0f);
            }
            break;
        }
        case EditorSection::cab:
        {
            graphics.drawRoundedRectangle(iconBounds.reduced(0.5f), 2.0f, 1.6f);
            graphics.drawEllipse(iconBounds.withSizeKeepingCentre(12.0f, 12.0f), 1.6f);
            graphics.drawEllipse(iconBounds.withSizeKeepingCentre(5.0f, 5.0f), 1.2f);
            break;
        }
        case EditorSection::count:
            break;
    }
}

} // namespace lumen::ui
