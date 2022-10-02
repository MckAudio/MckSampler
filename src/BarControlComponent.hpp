#pragma once
#include <JuceHeader.h>

class ControlComponent : public juce::Component
{
    public:
    ControlComponent() {
        bspSlider.setSliderStyle(juce::Slider::RotaryVerticalDrag);
        bspSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 90, 40);
        bspSlider.setPopupDisplayEnabled(true, true, this, 1000);
        addAndMakeVisible(bspSlider);
    }

    void paint(juce::Graphics& g) override
    {
        g.fillAll(juce::Colours::mistyrose);
    }

    void resized() override
    {
        bspSlider.setBounds(0, 40, 80, 80);
    }

    private:
    juce::Slider bspSlider;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ControlComponent)
};