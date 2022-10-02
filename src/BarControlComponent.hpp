#pragma once
#include <JuceHeader.h>

class ControlComponent : public juce::Component
{
    public:
    ControlComponent() {}

    void paint(juce::Graphics& g) override
    {
        g.fillAll(juce::Colours::mistyrose);
    }

    private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ControlComponent);
};