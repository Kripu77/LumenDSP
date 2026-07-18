#include "ui/KnobComponent.h"

namespace lumen::ui
{

namespace
{

class ValueLabelUpdater : public juce::Slider::Listener
{
public:
    explicit ValueLabelUpdater(std::function<void()> callback)
        : onChange(std::move(callback))
    {
    }

    void sliderValueChanged(juce::Slider*) override
    {
        if (onChange)
            onChange();
    }

private:
    std::function<void()> onChange;
};

} // namespace

KnobComponent::KnobComponent(const juce::String& labelText, const juce::String& valueSuffix, Style style)
    : visualStyle(style)
    , suffix(valueSuffix)
{
    slider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    slider.setTextBoxStyle(juce::Slider::NoTextBox, true, 0, 0);
    slider.setRotaryParameters(design::knobStartAngleRadians, design::knobEndAngleRadians, true);
    slider.setMouseDragSensitivity(static_cast<int>(design::knobDragSensitivityPixelsPerFullRange));
    setStyle(style);
    addAndMakeVisible(slider);

    titleLabel.setText(labelText.toUpperCase(), juce::dontSendNotification);
    titleLabel.setJustificationType(juce::Justification::centred);
    titleLabel.setFont(design::microFont());
    addAndMakeVisible(titleLabel);

    valueLabel.setJustificationType(juce::Justification::centred);
    valueLabel.setFont(design::valueFont());
    addAndMakeVisible(valueLabel);

    valueListener = std::make_unique<ValueLabelUpdater>([this]() { updateValueLabel(); });
    slider.addListener(valueListener.get());
    updateValueLabel();
}

KnobComponent::~KnobComponent()
{
    if (valueListener != nullptr)
        slider.removeListener(valueListener.get());
}

void KnobComponent::resized()
{
    auto bounds = getLocalBounds();
    titleLabel.setBounds(bounds.removeFromTop(14));
    valueLabel.setBounds(bounds.removeFromBottom(14));
    slider.setBounds(bounds.reduced(2));
}

void KnobComponent::paint(juce::Graphics&)
{
}

juce::Slider& KnobComponent::getSlider() noexcept
{
    return slider;
}

void KnobComponent::setStyle(Style style)
{
    visualStyle = style;
    slider.getProperties().set("metalStyle", true);
    titleLabel.setColour(juce::Label::textColourId, design::textMuted());
    valueLabel.setColour(juce::Label::textColourId, design::textTertiary());
    slider.repaint();
}

void KnobComponent::setDisplayDecimals(int decimalPlaces)
{
    decimals = juce::jmax(0, decimalPlaces);
    updateValueLabel();
}

void KnobComponent::updateValueLabel()
{
    valueLabel.setText(formatValue(slider.getValue()), juce::dontSendNotification);
}

juce::String KnobComponent::formatValue(double value) const
{
    juce::String formatted = juce::String(value, decimals);
    if (suffix.isNotEmpty())
        formatted << " " << suffix;
    return formatted;
}

} // namespace lumen::ui
