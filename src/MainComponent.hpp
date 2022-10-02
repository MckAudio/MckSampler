#pragma once
#include <JuceHeader.h>
#include "BarControlComponent.hpp"
#include "BarMenuComponent.hpp"
#include "BarPadComponent.hpp"
#include "BarSelectorComponent.hpp"

class MainComponent : public juce::Component
{
    public:

    MainComponent()
    {
        setSize(defaultWidth, defaultHeight);

        addAndMakeVisible(ctrlComponent);
        addAndMakeVisible(menuComponent);
        addAndMakeVisible(padComponent);
        addAndMakeVisible(selComponent);
    }

    void paint(juce::Graphics& g) override {
        g.setFont(64);
        g.setColour(juce::Colours::lightseagreen);
        g.drawText(currentSizeAsString, getLocalBounds(), juce::Justification::centred, true);
    }
    
    void resized() override
    {
        currentSizeAsString = juce::String(getWidth()) + " x " + juce::String(getHeight());

        menuComponent.setBounds(0,0, bigUnit, getHeight());
        selComponent.setBounds(bigUnit, 0, getWidth() - 2 * bigUnit, smallUnit);
        ctrlComponent.setBounds(getWidth() - bigUnit, 0, bigUnit, getHeight());
        padComponent.setBounds(bigUnit, getHeight() - bigUnit, getWidth() - 2 * bigUnit, bigUnit);
    }

    private:
    const int defaultHeight { 480 };
    const int defaultWidth { 800 };
    const int bigUnit { 80 };
    const int smallUnit { 40 };
    const int rows { defaultHeight / bigUnit };
    const int cols { defaultWidth / bigUnit };
    juce::String currentSizeAsString;

    ControlComponent ctrlComponent;
    MenuComponent menuComponent;
    PadComponent padComponent;
    SelectorComponent selComponent;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainComponent)
};