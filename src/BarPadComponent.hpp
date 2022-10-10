#pragma once
#include <JuceHeader.h>
#include "Content.hpp"
#include "Processing.hpp"

class PadComponent : public ControlComponentBase, public juce::Button::Listener
{
    public:
    PadComponent() {
        for(auto&b : buttons)
        {
            b.addListener(this);
            addAndMakeVisible(b);
        }
    }

    void paint(juce::Graphics& g) override
    {
        auto area = getLocalBounds();
        auto h = area.getHeight();
        g.fillAll(getLookAndFeel().findColour(juce::DocumentWindow::backgroundColourId));
        g.setColour(getLookAndFeel().findColour(juce::ComboBox::outlineColourId));
        area.removeFromBottom(h - 1);
        g.fillRect(area);


        for (size_t i = 1; i < numPads; i++)
        {
            g.fillRect(i*itemWidth, margin, 1, h - 2*margin);
        }
    }

    void resized() override {
        auto area = getLocalBounds();

        for (size_t i = 0; i < numPads; i++)
        {
            buttons[i].setBounds(i*itemWidth + margin, margin, buttonSize, buttonSize);
        }
    }

    private:
        void buttonClicked (Button *b) override {
            for (size_t i = 0; i < 8; i++)
            {
                if (b == buttons + i)
                {
                    mck::Processing::GetInstance()->Trigger(i, 1.0);
                }
            }
        }
    static const size_t numPads{8};
    static const int margin{8};
    static const int buttonSize{64};
    static const int itemWidth{buttonSize + 2 * margin};
    juce::TextButton buttons[numPads];

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PadComponent)
};