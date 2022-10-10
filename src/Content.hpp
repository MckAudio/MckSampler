#pragma once
#include <JuceHeader.h>
#include "Dial.hpp"
#include "Processing.hpp"

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

    enum Ctrl
    {
        Pan = 0,
        Pitch,
        Reverb,
        Level
    };
};

class ControlComponentBase : public juce::Component
{
public:
    virtual void setActiveContent(Content::Type contentType)
    {
        activeContentType = contentType;
    }

protected:
    Content::Type activeContentType{Content::Controls};
};

class ControlPageComponent : public juce::Component, public juce::Slider::Listener, public mck::Processing::Listener
{
public:
    ControlPageComponent();
    ~ControlPageComponent();

    void paint(juce::Graphics &g) override;
    void resized() override;

private:
    void sliderValueChanged(Slider *slider) override;
    void configChanged(const mck::sampler::Config &config) override;
};

class MixerComponent : public juce::Component,
                       public juce::Slider::Listener,
                       public mck::Processing::Listener
{
public:
    MixerComponent();
    ~MixerComponent();

    void paint(juce::Graphics &g) override;
    void resized() override;

private:
    void sliderValueChanged(Slider *slider) override;

    void configChanged(const mck::sampler::Config &config) override;

    const size_t numCtrls{4};
    const size_t numChannels{8};
    const size_t numDials{numCtrls * numChannels};
    const int labelHeight{20};
    const int upperRowMargin{4};
    const int lowerRowMargin{8};
    const int colMargin{8};
    const int dialHeight{64};
    const int fontSize{14};
    const int rowWidth{80};
    const int rowHeight{upperRowMargin + dialHeight + fontSize + lowerRowMargin};

    Dial dials[32];
    juce::Label labels[32];
    juce::String labelTexts[4] = {
        "Pan", "Pitch", "Reverb", "Level"};
};

class PadsComponent : public juce::Component, public juce::Button::Listener
{
public:
    PadsComponent()
    {
        auto conf = mck::Processing::GetInstance()->GetCurrentConfig();
        size_t i = 0;
        for (auto &p : pads)
        {
            if (i < conf.numPads)
            {
                p.setButtonText(conf.pads[i].sampleName);
            }
            else
            {
                p.setButtonText("Pad #" + std::to_string(i + 1));
            }
            p.addListener(this);
            addAndMakeVisible(p);
            i++;
        }
    }

    void resized() override
    {
        auto area = getLocalBounds();
        auto w = area.getWidth() / numCols;
        auto h = area.getHeight() / numRows;

        size_t pIdx = 0;
        for (int r = 0; r < numRows; r++)
        {
            for (int c = 0; c < numCols; c++, pIdx++)
            {
                pads[pIdx].setBounds(juce::Rectangle<int>(c * w, r * h, w, h).reduced(margin));
            }
        }
    }

private:
    void buttonClicked(Button *b) override;

    const static size_t numPads{8};
    const static size_t numRows{2};
    const static size_t numCols{4};
    const static int margin{8};

    juce::TextButton pads[numPads];
};