#pragma once
#include <JuceHeader.h>
#include "Content.hpp"
#include "Processing.hpp"

class SelectorComponent : public ControlComponentBase, public juce::Button::Listener, public mck::Processing::Listener
{
public:
    SelectorComponent()
    {
        mck::Processing::GetInstance()->addListener(this);
    }
    ~SelectorComponent()
    {
        mck::Processing::GetInstance()->removeListener(this);
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
            g.fillRect(i * itemWidth, margin, 1, h - 2 * margin);
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
        case Content::Samples:
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

        auto conf = mck::Processing::GetInstance()->GetCurrentConfig();
        size_t i = 0;
        for (auto &b : buttons)
        {
            if (i < conf.numPads && conf.pads[i].sampleName != "")
            {
                b.setButtonText(conf.pads[i].sampleName);
            }
            else
            {
                b.setButtonText("Pad #" + std::to_string(i + 1));
            }
            b.setToggleable(true);
            b.addListener(this);
            addAndMakeVisible(b);

            i++;
        }
        drawLabels = false;
        drawButtons = true;
        resized();
    }

    void addLabels()
    {
        removeAllChildren();

        auto conf = mck::Processing::GetInstance()->GetCurrentConfig();
        size_t i = 0;
        for (auto &l : labels)
        {
            l.setFont(juce::Font(16, juce::Font::plain));
            if (i < conf.numPads && conf.pads[i].sampleName != "")
            {
                l.setText(conf.pads[i].sampleName, juce::NotificationType::dontSendNotification);
            }
            else
            {
                l.setText("Pad #" + std::to_string(i + 1), juce::NotificationType::dontSendNotification);
            }
            l.setJustificationType(juce::Justification::centred);
            addAndMakeVisible(l);
            i++;
        }
        drawButtons = false;
        drawLabels = true;
        resized();
    }

    void buttonClicked(juce::Button *b) override
    {

        for (size_t i = 0; i < 8; i++)
        {
            //buttons[i].setToggleState(false, false);
            if (b == &buttons[i])
            {
                //buttons[i].setToggleState(true, false);
                mck::Processing::GetInstance()->SetActivePad(i);
            }
        }
    }

    void configChanged(const mck::sampler::Config &config) override
    {
        for (size_t i = 0; i < std::min(config.numPads, static_cast<const unsigned>(8)); i++)
        {
            if (i == config.activePad) {
                buttons[i].setToggleState(true, false);
            } else {
                buttons[i].setToggleState(false, false);
            }
        }
    }

    juce::TextButton buttons[8];
    juce::Label labels[8];

    bool drawLabels{false};
    bool drawButtons{false};

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SelectorComponent)
};