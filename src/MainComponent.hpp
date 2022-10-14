#pragma once
#include <JuceHeader.h>
#include "BarControlComponent.hpp"
#include "BarMenuComponent.hpp"
#include "BarPadComponent.hpp"
#include "BarSelectorComponent.hpp"
#include "Content.hpp"

class MainComponent : public juce::Component, public MenuComponent::Listener
{
public:
    MainComponent()
    {
        setSize(defaultWidth, defaultHeight);

        menuComponent.addListener(this);

        addAndMakeVisible(ctrlComponent);
        addAndMakeVisible(menuComponent);
        addAndMakeVisible(padComponent);
        addAndMakeVisible(selComponent);

        // addAndMakeVisible(mixerComponent);

        setActiveContent(Content::Mixer);

        resized();
    }

    ~MainComponent()
    {
        if (contentComponent != nullptr)
        {
            delete contentComponent;
            contentComponent = nullptr;
        }
    }

    void paint(juce::Graphics &g) override
    {
        g.fillAll(getLookAndFeel().findColour(juce::ComboBox::outlineColourId));
        auto area = getLocalBounds();
        area.reduced(1);
        g.setColour(getLookAndFeel().findColour(juce::DocumentWindow::backgroundColourId));
        g.fillRect(area);
    }

    void resized() override
    {
        currentSizeAsString = juce::String(getWidth()) + " x " + juce::String(getHeight());

        menuComponent.setBounds(0, 0, bigUnit, getHeight());
        selComponent.setBounds(bigUnit, 0, getWidth() - 2 * bigUnit, smallUnit);
        ctrlComponent.setBounds(getWidth() - bigUnit, 0, bigUnit, getHeight());
        padComponent.setBounds(bigUnit, getHeight() - bigUnit, getWidth() - 2 * bigUnit, bigUnit);

        if (contentComponent != nullptr)
        {
            contentComponent->setBounds(bigUnit, smallUnit, defaultWidth - 2 * bigUnit, defaultHeight - bigUnit - smallUnit);
        }
    }

    void setActiveContent(Content::Type contentType)
    {
        auto sc = dynamic_cast<SampleComponent *>(contentComponent);
        if (sc != nullptr) {
            sampleComponentState = sc->getState();
        }
        removeChildComponent(contentComponent);
        if (contentComponent != nullptr)
        {
            delete contentComponent;
            contentComponent = nullptr;
        }

        switch (contentType)
        {
        case Content::Controls:
            contentComponent = new ControlPageComponent();
            addAndMakeVisible(contentComponent);
            break;
        case Content::Samples:
            contentComponent = new SampleComponent();
            static_cast<SampleComponent *>(contentComponent)->setState(sampleComponentState);
            addAndMakeVisible(contentComponent);
            break;
        case Content::Mixer:
            contentComponent = new MixerComponent();
            addAndMakeVisible(contentComponent);
            break;
        case Content::Pads:
            contentComponent = new PadsComponent();
            addAndMakeVisible(contentComponent);
            break;

        default:
            break;
        }
        activeContent = contentType;
        selComponent.setActiveContent(contentType);
        menuComponent.setActiveContent(contentType);

        resized();
    }

    void menuItemChanged(int idx) override
    {
        std::cout << "Menu item: " << idx << std::endl;
        if (idx >= Content::Type::Controls && idx < Content::Type::Length)
        {
            setActiveContent(static_cast<Content::Type>(idx));
        }
    }

private:
    Content::Type activeContent{Content::Type::Mixer};

    const int defaultHeight{480};
    const int defaultWidth{800};
    const int bigUnit{80};
    const int smallUnit{40};
    const int rows{defaultHeight / bigUnit};
    const int cols{defaultWidth / bigUnit};
    juce::String currentSizeAsString;

    ControlComponent ctrlComponent;
    MenuComponent menuComponent;
    PadComponent padComponent;
    SelectorComponent selComponent;

    // MixerComponent mixerComponent;
    juce::Component *contentComponent{nullptr};

    SampleComponent::State sampleComponentState{};

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainComponent)
};