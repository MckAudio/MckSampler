#pragma once

#include <JuceHeader.h>
#include <atomic>

class DrumPadComponent : public juce::Component, public juce::Timer
{
public:
    DrumPadComponent()
    {
        startTimerHz(60);
    }
    ~DrumPadComponent()
    {
        stopTimer();
    }

    void paint(juce::Graphics &g)
    {
        auto area = getLocalBounds();
        g.setColour(getLookAndFeel().findColour(juce::TextButton::buttonColourId));
        g.fillRoundedRectangle(area.toFloat(), 8.0f);
        g.setColour(getLookAndFeel().findColour(juce::DocumentWindow::backgroundColourId));
        g.fillRoundedRectangle(area.reduced(1).toFloat(), 8.0f);

        if (paintPoint)
        {
            auto pos = Point<float>(valX.load() * area.getWidth(), valY.load() * area.getHeight());
            auto gr = std::min(area.getWidth(), area.getHeight()) / 2.0f;
            auto pos2 = pos + Point<float>(gr, gr);
            auto grad = ColourGradient(juce::Colours::white, pos, juce::Colours::transparentWhite, pos2, true);
            g.setGradientFill(grad);
            g.fillRoundedRectangle(area.reduced(1).toFloat(), 8.0f);
        }
        else if (pointStrength >= 0.0f)
        {
            auto pos = Point<float>(valX.load() * area.getWidth(), valY.load() * area.getHeight());
            auto gr = std::min(area.getWidth(), area.getHeight()) / 2.0f * pointStrength;
            auto pos2 = pos + Point<float>(gr, gr);
            auto grad = ColourGradient(juce::Colours::white, pos, juce::Colours::transparentWhite, pos2, true);
            g.setGradientFill(grad);
            g.fillRoundedRectangle(area.reduced(1).toFloat(), 8.0f);

            pointStrength -= 0.5/60.0;
        }
    }

    void resized()
    {
    }

    void mouseDown(const MouseEvent &e) override
    {
        auto area = getLocalBounds().toDouble();
        valX = static_cast<double>(e.x) / area.getWidth();
        valY = static_cast<double>(e.y) / area.getHeight();
        paintPoint = true;

        if (e.source.getType() == MouseInputSource::touch) {
            std::printf("Touch device with finger #%d!\n", e.source.getIndex());
        } else if (e.source.getType() == MouseInputSource::mouse) {
            std::printf("Mouse device!\n");
        }
    }
    void mouseDrag(const MouseEvent &e) override
    {
        auto area = getLocalBounds().toDouble();
        valX = static_cast<double>(e.x) / area.getWidth();
        valY = static_cast<double>(e.y) / area.getHeight();
        paintPoint = true;
        if (e.source.getType() == MouseInputSource::touch) {
            std::printf("Touch move with finger #%d!\n", e.source.getIndex());
        }
    }
    void mouseUp(const MouseEvent &e) override
    {
        paintPoint = false;
        pointStrength = 1.0;
    }
    void timerCallback() override
    {
        repaint();
    }

private:
    bool paintPoint{false};
    double pointStrength{0.0};
    std::atomic<double> valX{0.0};
    std::atomic<double> valY{0.0};

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DrumPadComponent);
};