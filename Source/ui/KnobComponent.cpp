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

KnobComponent::KnobComponent(const juce::String& labelText, const juce::String& valueSuffix)
    : suffix(valueSuffix)
{
    slider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    slider.setTextBoxStyle(juce::Slider::NoTextBox, true, 0, 0);
    slider.setRotaryParameters(
        design::knobStartAngleRadians,
        design::knobEndAngleRadians,
        true);
    slider.setMouseDragSensitivity(static_cast<int>(design::knobDragSensitivityPixelsPerFullRange));
    addAndMakeVisible(slider);

    titleLabel.setText(labelText, juce::dontSendNotification);
    titleLabel.setJustificationType(juce::Justification::centred);
    titleLabel.setColour(juce::Label::textColourId, design::textSecondary());
    titleLabel.setFont(design::sectionFont());
    addAndMakeVisible(titleLabel);

    valueLabel.setJustificationType(juce::Justification::centred);
    valueLabel.setColour(juce::Label::textColourId, design::textPrimary());
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
    titleLabel.setBounds(bounds.removeFromTop(labelHeightPixels));
    valueLabel.setBounds(bounds.removeFromBottom(valueHeightPixels));
    bounds.reduce(0, sliderVerticalPaddingPixels);
    slider.setBounds(bounds);
}

void KnobComponent::paint(juce::Graphics& graphics)
{
    juce::ignoreUnused(graphics);
}

juce::Slider& KnobComponent::getSlider() noexcept
{
    return slider;
}

void KnobComponent::setAnimatedValue(double targetValue, bool animate)
{
    if (!animate)
    {
        slider.setValue(targetValue, juce::sendNotificationSync);
        return;
    }

    slider.setValue(targetValue, juce::sendNotificationSync);
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
