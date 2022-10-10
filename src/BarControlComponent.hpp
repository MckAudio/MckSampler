#pragma once
#include <JuceHeader.h>
#include "Dial.hpp"
#include "Content.hpp"

class ControlComponent : public ControlComponentBase
{
    public:
    ControlComponent() {
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
    }

    private:

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ControlComponent)
};