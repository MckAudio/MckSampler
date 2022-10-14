#pragma once
#include <JuceHeader.h>
#include "Dial.hpp"
#include "Content.hpp"
#include "LevelMeter.hpp"

class ControlComponent : public ControlComponentBase
{
    public:
    ControlComponent() {
        addAndMakeVisible(levelMeter);
    }

    void paint(juce::Graphics& g) override
    {
        auto area = getLocalBounds();
        g.fillAll(getLookAndFeel().findColour(juce::DocumentWindow::backgroundColourId));
        g.setColour(getLookAndFeel().findColour(juce::ComboBox::outlineColourId));
        area.removeFromRight(area.getWidth() - 1);
        g.fillRect(area);

        area = getLocalBounds();
        area.removeFromTop(40);
        area.setHeight(1);
        g.fillRect(area);
        area = getLocalBounds();
        area.removeFromTop(area.getHeight()-80);
        area.setHeight(1);
        g.fillRect(area);
    }

    void resized() override
    {
        auto area = getLocalBounds();
        area.removeFromTop(80);
        area.removeFromBottom(80);
        levelMeter.setBounds(area.reduced(8));
    }

    private:
    LevelMeterComponent levelMeter;


    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ControlComponent)
};