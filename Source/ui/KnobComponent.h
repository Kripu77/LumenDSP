#pragma once

#include <JuceHeader.h>
#include "ui/DesignTokens.h"

namespace lumen::ui
{

class KnobComponent : public juce::Component
{
public:
    enum class Style
    {
        chrome,
        metal
    };

    KnobComponent(const juce::String& labelText, const juce::String& valueSuffix, Style style = Style::chrome);
    ~KnobComponent() override;

    void resized() override;
    void paint(juce::Graphics& graphics) override;

    juce::Slider& getSlider() noexcept;
    void setStyle(Style style);
    void setDisplayDecimals(int decimalPlaces);

private:
    static constexpr int labelHeightPixels = 16;
    static constexpr int valueHeightPixels = 15;
    static constexpr int sliderVerticalPaddingPixels = 2;

    void updateValueLabel();
    juce::String formatValue(double value) const;

    Style visualStyle;
    juce::String suffix;
    int decimals = 1;
    juce::Slider slider;
    juce::Label titleLabel;
    juce::Label valueLabel;
    std::unique_ptr<juce::Slider::Listener> valueListener;
};

} // namespace lumen::ui
