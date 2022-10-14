#pragma once

#include <JuceHeader.h>
#include <MckHelper/DspHelper.hpp>
#include "Processing.hpp"

class LevelMeterComponent : public juce::Component, public juce::Timer
{
public:
    LevelMeterComponent(int borderSize = 1, float borderRadius = 0.0f)
        : m_borderRadius{borderRadius},
          m_borderSize{borderSize}
    {
        startTimerHz(60);
    };
    ~LevelMeterComponent()
    {
        if (isTimerRunning())
        {
            stopTimer();
        }
    }

    void paint(juce::Graphics &g) override
    {
        auto area = getLocalBounds();

        g.setColour(getLookAndFeel().findColour(juce::ComboBox::outlineColourId));
        g.fillRoundedRectangle(area.toFloat(), m_borderRadius);
        g.setColour(getLookAndFeel().findColour(juce::DocumentWindow::backgroundColourId));
        g.fillRoundedRectangle(area.reduced(m_borderSize).toFloat(), m_borderRadius);

        area.reduce(m_borderSize,m_borderSize);
        auto h = area.getHeight();
        auto w = area.getWidth() / 2;


        auto level = mck::Processing::GetInstance()->GetLevelDb();

        auto ll = static_cast<int>(std::round(mck::DbToLog(level.left) * static_cast<double>(h)));
        auto lr = static_cast<int>(std::round(mck::DbToLog(level.right) * static_cast<double>(h)));

        auto gl = juce::ColourGradient::vertical(juce::Colours::red, area.getY(), juce::Colours::green, area.getBottom());
        auto gr = juce::ColourGradient::vertical(juce::Colours::coral, area.getY(), juce::Colours::seagreen, area.getBottom());
        g.setGradientFill(gl);
        g.fillRoundedRectangle(area.getX(), h - ll, w - 1, ll, m_borderRadius);
        g.fillRoundedRectangle(area.getX() + w + 1, h - lr, w - 1, lr, m_borderRadius);
    }

private:
    void timerCallback() override
    {
        repaint();
    }

    float m_borderRadius{0.0f};
    int m_borderSize{1};
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(LevelMeterComponent)
};