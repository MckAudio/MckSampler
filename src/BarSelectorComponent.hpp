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
    ~SelectorComponent() override
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

        for (int i = 1; i < static_cast<int>(numPads); i++)
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
        case Content::Length:
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
            b.setToggleable(true);
            b.addListener(this);
            addAndMakeVisible(b);

            i++;
        }
        drawLabels = false;
        drawButtons = true;
        update();
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
            // buttons[i].setToggleState(false, false);
            if (b == &buttons[i])
            {
                // buttons[i].setToggleState(true, false);
                mck::Processing::GetInstance()->SetActivePad(i);
            }
        }
    }

    void update()
    {
        std::string tmp;
        for (size_t i = 0; i < std::min(processingConfig.numPads, static_cast<const unsigned>(8)); i++)
        {
            // Set Labels
            tmp = "0" + std::to_string(i + 1);
            auto sampleId = processingConfig.pads[i].sampleId;
            if (i < processingConfig.numPads && sampleId != "")
            {
                tmp = tmp + " - " + processingConfig.pads[i].sampleType;
            }
            labels[i].setText(tmp, juce::NotificationType::dontSendNotification);

            // Set Buttons
            buttons[i].setButtonText(tmp);
            if (i == processingConfig.activePad)
            {
                buttons[i].setToggleState(true, NotificationType::dontSendNotification);
            }
            else
            {
                buttons[i].setToggleState(false, NotificationType::dontSendNotification);
            }
        }
    }

    void configChanged(const mck::sampler::Config &config) override
    {
        processingConfig = config;
        update();
    }

    juce::TextButton buttons[8];
    juce::Label labels[8];

    bool drawLabels{false};
    bool drawButtons{false};

    mck::sampler::Config processingConfig{};
    std::vector<mck::SamplePack> samplePacks{};

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SelectorComponent)
};
