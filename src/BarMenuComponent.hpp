#pragma once
#include <JuceHeader.h>

class MenuComponent : public juce::Component
{
    public:
    MenuComponent() {}

    void paint(juce::Graphics& g) override
    {
        g.fillAll(juce::Colours::bisque);
    }

    private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MenuComponent)
};