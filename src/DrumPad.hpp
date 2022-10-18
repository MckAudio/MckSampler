#pragma once

#include <JuceHeader.h>
#include <atomic>

class DrumPadComponent : public juce::Component
{
public:
    DrumPadComponent(bool twoStep = false)
        : twoStepMode{twoStep}
    {
    }
    ~DrumPadComponent()
    {
    }

    void paint(juce::Graphics &g)
    {
        auto area = getLocalBounds();
        g.setColour(getLookAndFeel().findColour(juce::ComboBox::outlineColourId));
        g.fillRoundedRectangle(area.toFloat(), 8.0f);
        g.setColour(getLookAndFeel().findColour(juce::TextButton::buttonColourId));
        g.fillRoundedRectangle(area.reduced(1).toFloat(), 8.0f);

        auto pos = area.getCentre().toFloat();
        auto gr = pointStrength * std::min(area.getWidth(), area.getHeight()) / 2.0f;
        auto pos2 = pos + Point<float>(gr, gr);
        auto grad = ColourGradient(juce::Colours::white, pos, juce::Colours::transparentWhite, pos2, true);

        if (paintPoint)
        {
            g.setGradientFill(grad);
            g.fillRoundedRectangle(area.reduced(1).toFloat(), 8.0f);
        }
        else if (pointStrength > 0.0f)
        {
            g.setGradientFill(grad);
            g.fillRoundedRectangle(area.reduced(1).toFloat(), 8.0f);

            pointStrength -= 2.0/60.0;
            repaint();
        } else {
            pointStrength = 0.0;
            paintPoint = false;
        }

        g.setFont(16.0f);
        g.setColour(getLookAndFeel().findColour(juce::TextButton::textColourOffId));
        //g.drawMultiLineText(label, area.getX(), area.getY(), area.getWidth(), juce::Justification::centred);
        g.drawFittedText(label, area.reduced(4), juce::Justification::centred, 4);
    }

    void resized()
    {
    }

    void mouseDown(const MouseEvent &e) override
    {
        auto area = getLocalBounds().toDouble();
        valX = static_cast<double>(e.x) / area.getWidth();
        if (twoStepMode) {
            valY = e.y > area.getHeight() / 2 ? 0.7 : 1.0;
        } else {
            valY = 1.0 - static_cast<double>(e.y) / area.getHeight();
        }
        paintPoint = true;
        pointStrength = (0.2 + valY) / 1.2;
        listeners.call([this](Listener &l) {l.padDown(this, valY); });
        repaint();
    }
    void mouseDrag(const MouseEvent &e) override
    {
        /*std::printf("Touch move with finger #%d!\n", e.source.getIndex());
        }*/
    }
    void mouseUp(const MouseEvent &e) override
    {
        paintPoint = false;
        auto area = getLocalBounds().toDouble();
        if (twoStepMode) {
            valY = e.y > area.getHeight() / 2 ? 0.7 : 1.0;
        } else {
            valY = 1.0 - static_cast<double>(e.y) / area.getHeight();
        }
        listeners.call([this](Listener &l) {l.padUp(this, valY); });
        repaint();
    }

    void setPadText(const std::string &text) {
        label = text;
        repaint();
    }

    void setMode(bool twoStep) {
        twoStepMode = twoStep;
    }

    class JUCE_API Listener {
        public:
            virtual ~Listener() = default;

            virtual void padDown(DrumPadComponent *pad, double value) = 0;

            virtual void padUp(DrumPadComponent *pad, double value) {};
    };

    void addListener(Listener *newListener)
    {
        listeners.add(newListener);
    }

    void removeListener(Listener *listener) {
        listeners.remove(listener);
    }

private:
    bool twoStepMode{false};
    bool paintPoint{false};
    double pointStrength{0.0};
    std::atomic<double> valX{0.0};
    std::atomic<double> valY{0.0};
    std::string label{""};

    ListenerList<Listener> listeners;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DrumPadComponent)
};