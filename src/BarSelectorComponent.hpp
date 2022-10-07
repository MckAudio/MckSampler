#pragma once
#include <JuceHeader.h>
#include "Content.hpp"

class SelectorComponent : public ControlComponentBase, public juce::Button::Listener
{
public:
    SelectorComponent()
    {
    }

    void paint(juce::Graphics &g) override
    {
        auto area = getLocalBounds();
        auto h = area.getHeight();
        g.fillAll(getLookAndFeel().findColour(juce::DocumentWindow::backgroundColourId));
        g.setColour(getLookAndFeel().findColour(juce::ComboBox::outlineColourId));
        area.removeFromTop(h - 1);
        g.fillRect(area);

        for (size_t i = 1; i < numPads; i++)
        {
            g.fillRect(i*itemWidth, margin, 1, h - 2*margin);
        }
    }
    void resized() override
    {
        auto area = getLocalBounds();
        if (drawButtons)
        {
            for (auto &b : buttons)
            {
                b.setBounds(area.removeFromLeft(itemWidth).reduced(margin));
            }
        }
        else if (drawLabels)
        {
            for (auto &l : labels)
            {
                l.setBounds(area.removeFromLeft(itemWidth).reduced(margin));
            }
        }
    }

    void setActiveContent(Content::Type idx) override
    {
        switch (idx)
        {
        case Content::Pads:
        case Content::Mixer:
        case Content::Settings:
            addLabels();
            break;
        case Content::Controls:
            addButtons();
            break;
        default:
            removeAllChildren();
            drawButtons = false;
            drawLabels = false;
            resized();
            break;
        }
        activeContentType = idx;
    }

private:
    static const size_t numPads{8};
    static const int margin{8};
    static const int buttonSize{64};
    static const int itemWidth{buttonSize + 2 * margin};

    void addButtons()
    {
        removeAllChildren();

        int i = 1;
        for (auto &b : buttons)
        {
            b.setButtonText("Pad #" + std::to_string(i++));
            b.setToggleable(true);
            b.addListener(this);
            addAndMakeVisible(b);
        }
        drawLabels = false;
        drawButtons = true;
        resized();
    }

    void addLabels()
    {
        removeAllChildren();

        int i = 1;
        for (auto &l : labels)
        {
            l.setFont(juce::Font(16, juce::Font::plain));
            l.setText("Pad #" + std::to_string(i++), juce::NotificationType::dontSendNotification);
            l.setJustificationType(juce::Justification::centred);
            addAndMakeVisible(l);
        }
        drawButtons = false;
        drawLabels = true;
        resized();
    }

    void buttonClicked(juce::Button *b) override
    {
        for (auto &button : buttons)
        {
            button.setToggleState(false, false);
            if (b == &button)
            {
                button.setToggleState(true, false);
            }
        }
    }

    juce::TextButton buttons[8];
    juce::Label labels[8];

    bool drawLabels{false};
    bool drawButtons{false};

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SelectorComponent)
};