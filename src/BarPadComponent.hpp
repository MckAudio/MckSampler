#pragma once
#include <JuceHeader.h>
#include "Content.hpp"
#include "Processing.hpp"
#include "DrumPad.hpp"

class PadComponent : public ControlComponentBase, public DrumPadComponent::Listener
{
public:
    PadComponent()
    {
        for (auto &b : buttons)
        {
            b.setMode(true);
            b.addListener(this);
            addAndMakeVisible(b);
        }
    }

    void paint(juce::Graphics &g) override
    {
        auto area = getLocalBounds();
        auto h = area.getHeight();
        g.fillAll(getLookAndFeel().findColour(juce::DocumentWindow::backgroundColourId));
        g.setColour(getLookAndFeel().findColour(juce::ComboBox::outlineColourId));
        area.removeFromBottom(h - 1);
        g.fillRect(area);

        for (int i = 1; i < static_cast<int>(numPads); i++)
        {
            g.fillRect(i * itemWidth, margin, 1, h - 2 * margin);
        }
    }

    void resized() override
    {for (int i = 0; i < static_cast<int>(numPads); i++)
        {
            buttons[i].setBounds(i * itemWidth + margin, margin, buttonSize, buttonSize);
        }
    }

private:
    void padDown(DrumPadComponent *d, double strength) override
    {
        for (size_t i = 0; i < numPads; i++)
        {
            if (d == buttons + i)
            {
                mck::Processing::GetInstance()->Trigger(i, strength);
            }
        }
    }

    static const size_t numPads{8};
    static const int margin{8};
    static const int buttonSize{64};
    static const int itemWidth{buttonSize + 2 * margin};
    DrumPadComponent buttons[numPads];

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PadComponent)
};
