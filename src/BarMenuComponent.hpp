#pragma once
#include <JuceHeader.h>
#include <vector>
#include "Content.hpp"

class MenuComponent : public ControlComponentBase, public juce::Button::Listener
{
    public:
    MenuComponent() {
        int i = 0;
        for (auto &b : buttons)
        {
            b.setButtonText(items[i++]);
            b.setToggleable(true);
            b.addListener(this);
            addAndMakeVisible(b);
        }
    }

    void paint(juce::Graphics& g) override
    {
        auto area = getLocalBounds();
        g.fillAll(getLookAndFeel().findColour(juce::DocumentWindow::backgroundColourId));
        g.setColour(getLookAndFeel().findColour(juce::ComboBox::outlineColourId));
        area.removeFromLeft(area.getWidth() - 1);
        g.fillRect(area);
    }

    void resized() override
    {
        auto area = getLocalBounds();
        area.removeFromTop(40);
        for (auto &b : buttons)
        {
            b.setBounds(area.removeFromTop(40).reduced(8));
        }
    }

    void setActiveContent(Content::Type idx) override
    {
        for (auto &button : buttons)
        {
            button.setToggleState(false, false);
        }
        if (idx < 4) {
            buttons[idx].setToggleState(true, false);
        }
        activeContentType = idx;
    }

    class JUCE_API Listener
    {
        public:
        virtual ~Listener() = default;

        virtual void menuItemChanged(int idx) = 0;
    };

    void addListener(Listener* newListener) {menuListeners.add(newListener); };

    void removeListener(Listener* listener) {menuListeners.remove(listener); };


    private:
    static const size_t numMenuItems { 4 };

    ListenerList<Listener> menuListeners;


    void buttonClicked (juce::Button *b) override
    {
        auto bIdx = 0;
        for (size_t i = 0; i < numMenuItems; i++)
        {
            //button.setToggleState(false, false);
            if (b == buttons + i) {
                menuListeners.call([&i](Listener &l) {l.menuItemChanged(i); });
            }
        }
    }

    juce::String items[numMenuItems] = {"Controls", "Settings", "Pads", "Mixer"};
    juce::TextButton buttons[numMenuItems]; 

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MenuComponent)
};