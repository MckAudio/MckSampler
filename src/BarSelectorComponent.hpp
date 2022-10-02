#pragma once
#include <JuceHeader.h>

class SelectorComponent : public juce::Component
{
    public:
    SelectorComponent() {}

    void paint(juce::Graphics& g) override
    {
        g.fillAll(juce::Colours::blueviolet);
    }

    private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SelectorComponent);
};