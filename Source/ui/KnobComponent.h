#pragma once

#include <JuceHeader.h>
#include "ui/DesignTokens.h"

namespace lumen::ui
{

class KnobComponent : public juce::Component
{
public:
    KnobComponent(const juce::String& labelText, const juce::String& valueSuffix);
    ~KnobComponent() override;

    void resized() override;
    void paint(juce::Graphics& graphics) override;

    juce::Slider& getSlider() noexcept;
    void setAnimatedValue(double targetValue, bool animate);
    void setDisplayDecimals(int decimalPlaces);

private:
    static constexpr int labelHeightPixels = 18;
    static constexpr int valueHeightPixels = 16;
    static constexpr int sliderVerticalPaddingPixels = 4;

    void updateValueLabel();
    juce::String formatValue(double value) const;

    juce::String suffix;
    int decimals = 1;
    juce::Slider slider;
    juce::Label titleLabel;
    juce::Label valueLabel;
    std::unique_ptr<juce::Slider::Listener> valueListener;
};

} // namespace lumen::ui
