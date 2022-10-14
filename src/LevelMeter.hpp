#pragma once

#include <JuceHeader.h>
#include <MckHelper/DspHelper.hpp>
#include "Processing.hpp"

class LevelMeterComponent : public juce::Component, public juce::Timer
{
    public:
    LevelMeterComponent() {
        startTimerHz(60);
    };
    ~LevelMeterComponent() {
        if (isTimerRunning()) {
            stopTimer();
        }
    }
    
    void paint(juce::Graphics &g) override {
        auto area = getLocalBounds();
        auto h = area.getHeight();
        auto w = area.getWidth()/2;
        auto level = mck::Processing::GetInstance()->GetLevelDb();

        auto ll = static_cast<int>(std::round(mck::DbToLog(level.left) * static_cast<double>(h)));
        auto lr = static_cast<int>(std::round(mck::DbToLog(level.right) * static_cast<double>(h)));

        auto gl = juce::ColourGradient::vertical(juce::Colours::red, area.getY(), juce::Colours::green, area.getBottom());
        auto gr = juce::ColourGradient::vertical(juce::Colours::coral, area.getY(), juce::Colours::seagreen, area.getBottom());
        g.setGradientFill(gl);
        g.fillRect(0,h-ll, w-1, ll);
        g.fillRect(w+1,h-lr, w-1, lr);
    }

    private:
    void timerCallback() override {
        repaint();
    }

    double level{0.0};
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(LevelMeterComponent)
};