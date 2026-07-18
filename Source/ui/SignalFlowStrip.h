#pragma once

#include <JuceHeader.h>
#include "ui/DesignTokens.h"
#include "ui/SectionNavComponent.h"

namespace lumen::ui
{

class SignalFlowStrip : public juce::Component
{
public:
    SignalFlowStrip();

    void paint(juce::Graphics& graphics) override;
    void resized() override;
    void mouseUp(const juce::MouseEvent& event) override;

    void setActiveSection(EditorSection section);
    void setNodeStatus(EditorSection section, bool isActive, const juce::String& detail);
    EditorSection getActiveSection() const noexcept;

    std::function<void(EditorSection)> onSectionChanged;

private:
    struct Node
    {
        EditorSection section = EditorSection::input;
        juce::String title;
        juce::String detail;
        bool active = true;
        juce::Rectangle<float> bounds;
    };

    void layoutNodes();
    void drawNode(juce::Graphics& graphics, const Node& node, bool selected) const;
    juce::Colour colourForSection(EditorSection section) const;

    std::array<Node, static_cast<size_t>(EditorSection::count)> nodes{};
    EditorSection activeSection = EditorSection::amp;
};

} // namespace lumen::ui
