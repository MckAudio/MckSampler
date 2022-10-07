#pragma once
#include <JuceHeader.h>

class Dial : public juce::Slider
{
public:
    Dial()
    {
        setSliderStyle(RotaryVerticalDrag);
        setTextBoxStyle(NoTextBox, false, 90, 40);
        setPopupDisplayEnabled(true, true, getParentComponent(), 1000);
    }

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Dial)
};

class DialComponent : public juce::Component
{
public:
    DialComponent()
    {
        label.setFont(juce::Font(16, juce::Font::plain));
        label.setColour(juce::Label::textColourId, juce::Colour::fromRGB(230, 230, 230));
        label.setJustificationType(juce::Justification::centred);
        addAndMakeVisible(dial);
        addAndMakeVisible(label);
    }

    void setLabelText(juce::String labelText)
    {
        label.setText(labelText, juce::NotificationType::dontSendNotification);
    }

    void resized() override
    {
        auto area = getLocalBounds();
        int w = std::min(area.getWidth(), area.getHeight());
        //setBounds(area.getTopLeft().getX(), area.getTopLeft().getY(), w, w+20);
       //area = getLocalBounds();
        dial.setBounds(area.reduced(8));
        area.setTop(w);
        area.setBottom(w+20);
        label.setBounds(area);
    }

private:
    juce::Label label;
    Dial dial;
};