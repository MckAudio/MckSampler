#pragma once
#include <JuceHeader.h>
#include "Dial.hpp"
#include "Content.hpp"

class ControlComponent : public ControlComponentBase
{
    public:
    ControlComponent() {
        //bspSlider.setSliderStyle(juce::Slider::RotaryVerticalDrag);
        //bspSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 90, 40);
        //bspSlider.setPopupDisplayEnabled(true, true, this, 1000);
        bspSlider.setLabelText("Control");
        secSlider.setLabelText("Second");
        addAndMakeVisible(bspSlider);
        addAndMakeVisible(secSlider);
    }

    void paint(juce::Graphics& g) override
    {
        auto area = getLocalBounds();
        g.fillAll(getLookAndFeel().findColour(juce::DocumentWindow::backgroundColourId));
        g.setColour(getLookAndFeel().findColour(juce::ComboBox::outlineColourId));
        area.removeFromRight(area.getWidth() - 1);
        g.fillRect(area);
    }

    void resized() override
    {
        auto area = getLocalBounds();
        auto h = 80;
        auto m = 8;
        area.removeFromTop(h/2);
        bspSlider.setBounds(area.removeFromTop(h+20));
        secSlider.setBounds(area);
    }

    private:
    DialComponent bspSlider;
    DialComponent secSlider;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ControlComponent)
};