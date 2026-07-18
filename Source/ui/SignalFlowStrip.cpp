#include "ui/SignalFlowStrip.h"

namespace lumen::ui
{

SignalFlowStrip::SignalFlowStrip()
{
    stages[static_cast<size_t>(Stage::input)].label = "Input";
    stages[static_cast<size_t>(Stage::amp)].label = "Amp";
    stages[static_cast<size_t>(Stage::eq)].label = "EQ";
    stages[static_cast<size_t>(Stage::cab)].label = "Cab";
    stages[static_cast<size_t>(Stage::output)].label = "Output";

    for (auto& stage : stages)
        stage.active = true;
}

void SignalFlowStrip::paint(juce::Graphics& graphics)
{
    auto panelBounds = getLocalBounds().toFloat().reduced(0.5f);
    graphics.setColour(design::backgroundPanel().withAlpha(design::panelBackgroundAlpha));
    graphics.fillRoundedRectangle(panelBounds, static_cast<float>(design::panelCornerRadiusPixels));
    graphics.setColour(design::borderSubtle());
    graphics.drawRoundedRectangle(panelBounds, static_cast<float>(design::panelCornerRadiusPixels), 1.0f);

    for (size_t stageIndex = 0; stageIndex < stages.size(); ++stageIndex)
    {
        drawStage(graphics, stages[stageIndex]);

        if (stageIndex + 1 < stages.size())
        {
            drawConnector(
                graphics,
                stages[stageIndex].bounds,
                stages[stageIndex + 1].bounds,
                stages[stageIndex].active && stages[stageIndex + 1].active);
        }
    }
}

void SignalFlowStrip::resized()
{
    layoutStages();
}

void SignalFlowStrip::setStageActive(Stage stage, bool isActive)
{
    const auto index = static_cast<size_t>(stage);

    if (index >= stages.size())
        return;

    stages[index].active = isActive;
    repaint();
}

void SignalFlowStrip::setStageLabel(Stage stage, const juce::String& label)
{
    const auto index = static_cast<size_t>(stage);

    if (index >= stages.size())
        return;

    stages[index].label = label;
    repaint();
}

void SignalFlowStrip::layoutStages()
{
    auto content = getLocalBounds().reduced(design::spacingTwoUnitsPixels).toFloat();
    const float stageCount = static_cast<float>(stages.size());
    const float connectorWidth = static_cast<float>(design::signalFlowConnectorWidthPixels);
    const float totalConnectorWidth = connectorWidth * (stageCount - 1.0f);
    const float stageWidth = (content.getWidth() - totalConnectorWidth) / stageCount;
    const float stageHeight = static_cast<float>(design::signalFlowNodeHeightPixels);
    const float stageY = content.getCentreY() - stageHeight * 0.5f;

    float stageX = content.getX();

    for (auto& stage : stages)
    {
        stage.bounds = juce::Rectangle<float>(stageX, stageY, stageWidth, stageHeight);
        stageX += stageWidth + connectorWidth;
    }
}

void SignalFlowStrip::drawStage(juce::Graphics& graphics, const StageVisual& stageVisual) const
{
    auto fillColour = stageVisual.active ? design::backgroundElevated() : design::backgroundControl();
    auto borderColour = stageVisual.active ? design::accent() : design::borderSubtle();
    auto textColour = stageVisual.active ? design::textPrimary() : design::textMuted();

    if (stageVisual.active)
    {
        graphics.setColour(design::accent().withAlpha(design::signalFlowActiveGlowAlpha));
        graphics.fillRoundedRectangle(
            stageVisual.bounds.expanded(2.0f),
            static_cast<float>(design::controlCornerRadiusPixels));
    }

    graphics.setColour(fillColour);
    graphics.fillRoundedRectangle(
        stageVisual.bounds,
        static_cast<float>(design::controlCornerRadiusPixels));
    graphics.setColour(borderColour);
    graphics.drawRoundedRectangle(
        stageVisual.bounds,
        static_cast<float>(design::controlCornerRadiusPixels),
        1.2f);

    graphics.setColour(textColour);
    graphics.setFont(design::sectionFont());
    graphics.drawText(stageVisual.label, stageVisual.bounds, juce::Justification::centred);
}

void SignalFlowStrip::drawConnector(
    juce::Graphics& graphics,
    const juce::Rectangle<float>& fromBounds,
    const juce::Rectangle<float>& toBounds,
    bool active) const
{
    const float startX = fromBounds.getRight();
    const float endX = toBounds.getX();
    const float midY = fromBounds.getCentreY();
    const float arrowSize = 5.0f;

    graphics.setColour(active ? design::accent() : design::borderSubtle());
    graphics.drawLine(startX + 2.0f, midY, endX - arrowSize - 2.0f, midY, 1.5f);

    juce::Path arrow;
    arrow.addTriangle(
        endX - 2.0f,
        midY,
        endX - 2.0f - arrowSize,
        midY - arrowSize * 0.6f,
        endX - 2.0f - arrowSize,
        midY + arrowSize * 0.6f);
    graphics.fillPath(arrow);
}

} // namespace lumen::ui
