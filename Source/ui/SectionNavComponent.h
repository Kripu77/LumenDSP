#pragma once

#include <JuceHeader.h>
#include "ui/DesignTokens.h"

namespace lumen::ui
{

enum class EditorSection
{
    input = 0,
    amp,
    eq,
    cab,
    count
};

class SectionNavComponent : public juce::Component
{
public:
    SectionNavComponent();

    void paint(juce::Graphics& graphics) override;
    void resized() override;
    void mouseUp(const juce::MouseEvent& event) override;

    void setActiveSection(EditorSection section);
    EditorSection getActiveSection() const noexcept;

    std::function<void(EditorSection)> onSectionChanged;

private:
    struct NavItem
    {
        EditorSection section = EditorSection::input;
        juce::String title;
        juce::Rectangle<float> bounds;
    };

    void layoutItems();
    void drawItem(juce::Graphics& graphics, const NavItem& item, bool isActive) const;
    void drawGlyph(juce::Graphics& graphics, EditorSection section, juce::Rectangle<float> iconBounds, bool isActive) const;

    std::array<NavItem, static_cast<size_t>(EditorSection::count)> items{};
    EditorSection activeSection = EditorSection::amp;
};

} // namespace lumen::ui
