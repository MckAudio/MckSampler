#pragma once
#include <JuceHeader.h>

class PadComponent : public juce::Component
{
    public:
    PadComponent() {}

    void paint(juce::Graphics& g) override
    {
        g.fillAll(juce::Colours::dodgerblue);
    }

    private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PadComponent);
};