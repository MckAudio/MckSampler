#pragma once
#include <JuceHeader.h>
#include "Dial.hpp"

class Content
{
public:
    enum Type
    {
        Controls = 0,
        Settings,
        Pads,
        Mixer,
        Length
    };
};

class ControlComponentBase : public juce::Component
{
    public:
        virtual void setActiveContent(Content::Type contentType) {
            activeContentType = contentType;
        }
        
    protected:
        Content::Type activeContentType { Content::Controls };
};

class MixerComponent : public juce::Component
{
public:
    MixerComponent()
    {
        size_t d = 0;
        for (size_t i = 0; i < numChannels; i++)
        {
            for (size_t j = 0; j < numCtrls; j++, d++)
            {
                labels[d].setFont(juce::Font(fontSize, juce::Font::plain));
                labels[d].setJustificationType(juce::Justification::centred);
                labels[d].setText(labelTexts[j], juce::NotificationType::dontSendNotification);
                addAndMakeVisible(dials[d]);
                addAndMakeVisible(labels[d]);
            }
        }
    }
    void paint(juce::Graphics &g) override
    {
        // g.fillAll(juce::Colours::blanchedalmond);
        auto area = getLocalBounds();
        g.setColour(getLookAndFeel().findColour(juce::ComboBox::outlineColourId));
        for (size_t i = 0; i < numChannels; i++)
        {
            g.fillRect((i + 1) * rowWidth, upperRowMargin, 1, area.getHeight() - upperRowMargin - lowerRowMargin);
        }
    }
    void resized() override
    {
        int d = 0;
        for (size_t i = 0; i < numChannels; i++)
        {
            auto area = getLocalBounds();
            area.removeFromLeft(i * rowWidth);
            area.removeFromRight((numChannels - 1 - i) * rowWidth);

            for (size_t j = 0; j < numCtrls; j++, d++)
            {
                dials[d].setBounds(i * rowWidth + colMargin, j * rowHeight + upperRowMargin, dialHeight, dialHeight);
                labels[d].setBounds(i * rowWidth + colMargin, j * rowHeight + upperRowMargin + dialHeight, dialHeight, fontSize);
            }
        }
    }

private:
    const static size_t numCtrls{4};
    const static size_t numChannels{8};
    const static size_t numDials{numCtrls * numChannels};

    const static int labelHeight{20};
    const static int upperRowMargin{4};
    const static int lowerRowMargin{8};
    const static int colMargin{8};
    const static int dialHeight{64};
    const static int fontSize{14};
    const static int rowWidth{80};
    const static int rowHeight{upperRowMargin + dialHeight + fontSize + lowerRowMargin};

    Dial dials[numDials];
    juce::Label labels[numDials];
    juce::String labelTexts[numCtrls] = {
        "Pan", "Pitch", "Reverb", "Level"};
};

class PadsComponent : public juce::Component
{
public:
    PadsComponent()
    {
        int i = 1;
        for (auto &p : pads)
        {
            p.setButtonText("Pad #" + std::to_string(i++));
            addAndMakeVisible(p);
        }
    }

    void resized() override {
        auto area = getLocalBounds();
        auto w = area.getWidth() / numCols;
        auto h = area.getHeight() / numRows;

        size_t pIdx = 0;
        for (int r = 0; r < numRows; r++)
        {
            for (int c = 0; c < numCols; c++, pIdx++)
            {
                pads[pIdx].setBounds(juce::Rectangle<int>(c*w, r*h, w, h).reduced(margin));
            }
        }
    }

private:
    const static size_t numPads{8};
    const static size_t numRows{2};
    const static size_t numCols{4};
    const static int margin{8};

    juce::TextButton pads[numPads];
};