#include "ui/SignalFlowStrip.h"

namespace lumen::ui
{

SignalFlowStrip::SignalFlowStrip()
{
    nodes[static_cast<size_t>(EditorSection::input)] = {EditorSection::input, "INPUT", "Gain / Gate", true, {}};
    nodes[static_cast<size_t>(EditorSection::amp)] = {EditorSection::amp, "AMP", "NAM Model", false, {}};
    nodes[static_cast<size_t>(EditorSection::eq)] = {EditorSection::eq, "EQ", "Tone Stack", true, {}};
    nodes[static_cast<size_t>(EditorSection::cab)] = {EditorSection::cab, "CAB", "Impulse", false, {}};
}

void SignalFlowStrip::paint(juce::Graphics& graphics)
{
    auto bounds = getLocalBounds().toFloat();
    graphics.setColour(design::bgSecondary());
    graphics.fillRoundedRectangle(bounds, 10.0f);
    graphics.setColour(design::borderLight());
    graphics.drawRoundedRectangle(bounds.reduced(0.5f), 10.0f, 1.0f);

    for (size_t index = 0; index < nodes.size(); ++index)
    {
        if (index + 1 < nodes.size())
        {
            const auto from = nodes[index].bounds;
            const auto to = nodes[index + 1].bounds;
            const float y = from.getCentreY();
            const bool live = nodes[index].active && nodes[index + 1].active;
            graphics.setColour(live ? design::success().withAlpha(0.65f) : design::borderStrong());
            graphics.drawLine(from.getRight() + 2.0f, y, to.getX() - 2.0f, y, 2.0f);
            juce::Path arrow;
            arrow.addTriangle(to.getX() - 2.0f, y, to.getX() - 8.0f, y - 4.0f, to.getX() - 8.0f, y + 4.0f);
            graphics.fillPath(arrow);
        }

        drawNode(graphics, nodes[index], nodes[index].section == activeSection);
    }
}

void SignalFlowStrip::resized()
{
    layoutNodes();
}

void SignalFlowStrip::mouseUp(const juce::MouseEvent& event)
{
    if (!event.mouseWasClicked())
        return;

    for (const auto& node : nodes)
    {
        if (node.bounds.contains(event.position))
        {
            setActiveSection(node.section);
            if (onSectionChanged)
                onSectionChanged(node.section);
            break;
        }
    }
}

void SignalFlowStrip::setActiveSection(EditorSection section)
{
    activeSection = section;
    repaint();
}

void SignalFlowStrip::setNodeStatus(EditorSection section, bool isActive, const juce::String& detail)
{
    const auto index = static_cast<size_t>(section);
    if (index >= nodes.size())
        return;

    nodes[index].active = isActive;
    if (detail.isNotEmpty())
        nodes[index].detail = detail;
    repaint();
}

EditorSection SignalFlowStrip::getActiveSection() const noexcept
{
    return activeSection;
}

void SignalFlowStrip::layoutNodes()
{
    auto content = getLocalBounds().reduced(14, 12).toFloat();
    const float gap = 28.0f;
    const float nodeWidth = (content.getWidth() - gap * static_cast<float>(nodes.size() - 1)) / static_cast<float>(nodes.size());

    for (size_t index = 0; index < nodes.size(); ++index)
    {
        nodes[index].bounds = juce::Rectangle<float>(
            content.getX() + static_cast<float>(index) * (nodeWidth + gap),
            content.getY(),
            nodeWidth,
            content.getHeight());
    }
}

void SignalFlowStrip::drawNode(juce::Graphics& graphics, const Node& node, bool selected) const
{
    auto bounds = node.bounds;
    const auto colour = colourForSection(node.section);

    if (selected)
    {
        graphics.setColour(colour.withAlpha(0.16f));
        graphics.fillRoundedRectangle(bounds, 10.0f);
        graphics.setColour(colour);
        graphics.drawRoundedRectangle(bounds, 10.0f, 1.6f);
    }
    else
    {
        graphics.setColour(design::bgElevated());
        graphics.fillRoundedRectangle(bounds, 10.0f);
        graphics.setColour(design::borderLight());
        graphics.drawRoundedRectangle(bounds, 10.0f, 1.0f);
    }

    auto led = juce::Rectangle<float>(bounds.getX() + 10.0f, bounds.getCentreY() - 4.0f, 8.0f, 8.0f);
    graphics.setColour(node.active ? colour : design::textMuted());
    graphics.fillEllipse(led);
    if (node.active)
    {
        graphics.setColour(colour.withAlpha(0.3f));
        graphics.fillEllipse(led.expanded(3.0f));
    }

    auto text = bounds.reduced(24.0f, 8.0f);
    graphics.setColour(selected ? design::textPrimary() : design::textSecondary());
    graphics.setFont(design::sectionFont());
    graphics.drawText(node.title, text.removeFromTop(18.0f), juce::Justification::centredLeft);
    graphics.setColour(design::textMuted());
    graphics.setFont(design::microFont());
    graphics.drawFittedText(node.detail, text.toNearestInt(), juce::Justification::centredLeft, 1);
}

juce::Colour SignalFlowStrip::colourForSection(EditorSection section) const
{
    switch (section)
    {
        case EditorSection::input: return design::nodeInput();
        case EditorSection::amp: return design::nodeAmp();
        case EditorSection::eq: return design::nodeEq();
        case EditorSection::cab: return design::nodeCab();
        case EditorSection::count: break;
    }
    return design::textMuted();
}

} // namespace lumen::ui
