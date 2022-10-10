#include "Content.hpp"

// ControlPageComponent

ControlPageComponent::ControlPageComponent()
{
    mck::Processing::GetInstance()->addListener(this);
}
ControlPageComponent::~ControlPageComponent()
{
    mck::Processing::GetInstance()->removeListener(this);
}

void ControlPageComponent::paint(juce::Graphics &g)
{
}
void ControlPageComponent::resized()
{
}
void ControlPageComponent::configChanged(const mck::sampler::Config &config)
{
    std::printf("Active pad #%d\n", config.activePad + 1);
}

void ControlPageComponent::sliderValueChanged(Slider *slider)
{
}

MixerComponent::MixerComponent()
{
    size_t d = 0;
    for (size_t i = 0; i < numChannels; i++)
    {
        for (size_t j = 0; j < numCtrls; j++, d++)
        {
            switch (j)
            {
            case Content::Pan:
                dials[d].setTextValueSuffix("%");
                dials[d].setRange(-100.0, 100.0, 1.0);
                dials[d].setValue(0.0);
                dials[d].setDoubleClickReturnValue(true, 0.0);
                dials[d].setSkewFactor(1.0, true);
                break;
            case Content::Level:
                dials[d].setTextValueSuffix("dB");
                dials[d].setRange(-60.0, 6.0, 0.1);
                dials[d].setValue(0.0);
                dials[d].setSkewFactor(2.0);
                dials[d].setDoubleClickReturnValue(true, 0.0);
                break;
            default:
                break;
            }
            labels[d].setFont(juce::Font(fontSize, juce::Font::plain));
            labels[d].setJustificationType(juce::Justification::centred);
            labels[d].setText(labelTexts[j], juce::NotificationType::dontSendNotification);

            dials[d].addListener(this);
            addAndMakeVisible(dials[d]);
            addAndMakeVisible(labels[d]);
        }
    }

    mck::Processing::GetInstance()->addListener(this);
}

MixerComponent::~MixerComponent()
{
    mck::Processing::GetInstance()->removeListener(this);
    for (auto &d : dials)
    {
        d.removeListener(this);
    }
}

void MixerComponent::paint(juce::Graphics &g)
{
    // g.fillAll(juce::Colours::blanchedalmond);
    auto area = getLocalBounds();
    g.setColour(getLookAndFeel().findColour(juce::ComboBox::outlineColourId));
    for (size_t i = 0; i < numChannels; i++)
    {
        g.fillRect((i + 1) * rowWidth, upperRowMargin, 1, area.getHeight() - upperRowMargin - lowerRowMargin);
    }
}
void MixerComponent::resized()
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

void MixerComponent::configChanged(const mck::sampler::Config &config)
{
    size_t d = 0;
    for (size_t i = 0; i < std::min(static_cast<const size_t>(config.numPads), numChannels); i++)
    {
        for (size_t j = 0; j < numCtrls; j++, d++)
        {
            switch (j)
            {
            case Content::Pan:
                dials[d].setValue(config.pads[i].pan, juce::NotificationType::dontSendNotification);
                break;
            case Content::Level:
                dials[d].setValue(config.pads[i].gain, juce::NotificationType::dontSendNotification);
                break;
            default:
                break;
            }
        }
    }
}

void MixerComponent::sliderValueChanged(Slider *slider)
{
    for (size_t i = 0; i < numDials; i++)
    {
        if (slider == dials + i)
        {
            size_t padIdx = i / numCtrls;
            size_t ctrlIdx = i % numCtrls;
            switch (ctrlIdx)
            {
            case Content::Pan:
                mck::Processing::GetInstance()->SetPan(padIdx, slider->getValue());
                break;
            case Content::Level:
                mck::Processing::GetInstance()->SetLevel(padIdx, slider->getValue());
                break;
            default:
                break;
            }
        }
    }
}

void PadsComponent::buttonClicked(Button *b)
{
    for (size_t i = 0; i < numPads; i++)
    {
        if (b == pads + i)
        {
            mck::Processing::GetInstance()->Trigger(i, 1.0);
        }
    }
}