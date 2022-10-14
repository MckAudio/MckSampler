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
    }

    void resized() override
    {
        auto area = getLocalBounds();
        area.removeFromTop(40);
        area.removeFromBottom(80);
        levelMeter.setBounds(area.reduced(margin, 0));
    }

    private:
    LevelMeterComponent levelMeter{2, 8.0f};

    const int margin = 8;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ControlComponent)
};