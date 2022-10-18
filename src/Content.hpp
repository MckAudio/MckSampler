#pragma once
#include <JuceHeader.h>
#include "Dial.hpp"
#include "DrumPad.hpp"
#include "Processing.hpp"

class Content
{
public:
    enum Type
    {
        Controls = 0,
        Samples,
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

    void openButtonClicked();
    std::unique_ptr<juce::FileChooser> fileChooser;

    size_t activePad{0};

private:
    void sliderValueChanged(Slider *slider) override;
    void configChanged(const mck::sampler::Config &config) override;

    juce::TextButton openButton;

    DrumPadComponent pad;
};

class SampleListBox : public juce::Component, public juce::ListBoxModel
{
public:
    SampleListBox(bool multi)
        : multipleSelection(multi)
    {
        listBox.setMultipleSelectionEnabled(multipleSelection);
        listBox.setClickingTogglesRowSelection(multipleSelection);
        addAndMakeVisible(listBox);
    }

    int getNumRows() override
    {
        return listItems.size();
    }

    void paintListBoxItem(int rowNumber, Graphics &g, int width, int height, bool rowIsSelected) override
    {
        g.setColour(rowIsSelected ? getLookAndFeel().findColour(juce::TextButton::buttonOnColourId) : getLookAndFeel().findColour(juce::ListBox::textColourId));
        if (rowNumber < listItems.size())
        {
            g.drawText(listItems[rowNumber], 0, 0, width, height, juce::Justification::centred, true);
        }
    }

    void setActiveRow(int idx)
    {
        if (idx >= 0 && idx < getNumRows())
        {
            if (idx != listBox.getSelectedRow())
            {
                listBox.selectRow(idx);
            }
        }
        else
        {
            listBox.deselectAllRows();
        }
    }

    void listBoxItemClicked(int row, const MouseEvent&) override
    {
        std::printf("Last row selected: %d\n");

        if (multipleSelection)
        {
            auto sel = listBox.getSelectedRows();
            listSelection.resize(sel.size());

            std::printf("Selection: \n");
            for (int i = 0; i < sel.size(); i++)
            {
                listSelection[i] = sel[i];
                std::printf("%d: %s\n", sel[i], listItems[sel[i]].c_str());
            }
        }
        else
        {
            listSelection.clear();
            if (row >= 0)
            {
                listSelection.push_back(row);
            }
        }
        selectionListeners.call([this, &row](Listener &l)
                                { if (multipleSelection) {
                                    l.selectionChanged(this, listSelection);
     } else {
        l.selectionChanged(this, row);
     } });
    }

    void resized() override
    {
        listBox.setBounds(getLocalBounds());
    }

    void setItems(std::vector<std::string> &items)
    {
        listItems.clear();
        listBox.updateContent();
        listItems = items;
        listBox.updateContent();
    }

    class JUCE_API Listener
    {
    public:
        virtual ~Listener() = default;

        virtual void selectionChanged(SampleListBox *listBox, std::vector<int> &selection) = 0;

        virtual void selectionChanged(SampleListBox *listBox, int selection) = 0;
    };

    void addListener(Listener *newListener)
    {
        selectionListeners.add(newListener);
    }
    void removeListener(Listener *listener)
    {
        selectionListeners.remove(listener);
    }

private:
    const bool multipleSelection;
    juce::ListBox listBox{{}, this};
    std::vector<std::string> listItems;

    std::vector<int> listSelection;

    ListenerList<Listener> selectionListeners;
};

class SampleComponent : public juce::Component, public mck::Processing::Listener, public SampleListBox::Listener
{
public:
    struct SampleMeta
    {
        unsigned sampleIdx{0};
        unsigned packIdx{0};
        SampleMeta(){};
        SampleMeta(unsigned sampleIndex, unsigned packIndex) : sampleIdx{sampleIndex}, packIdx{packIndex} {};
    };
    struct State
    {
        bool init{false};

        int activePackIdx{-1};
        int activeCatIdx{-1};
        int activeSampleIdx{-1};

        std::string activePackName{""};
        std::string activeCatName{""};
        std::string activeSampleName{""};

        std::vector<std::string> packNames;
        std::vector<std::string> catNames;
        std::vector<std::string> sampleNames;
        std::vector<SampleMeta> sampleMetas;

        bool autoPlay{false};
    };

    SampleComponent();
    ~SampleComponent();

    void paint(juce::Graphics &g) override;
    void resized() override;

    State getState();
    void setState(State &newState);

private:
    void update();

    void updateControls();

    void updateCategories(int activePack);
    void updateSamples(int activePack, int activeCat);

    void previousSample();
    void nextSample();
    void playSample();
    void assignSample();

    void configChanged(const mck::sampler::Config &config) override;

    void samplesChanged(const std::vector<mck::SamplePack> &samples) override;

    void selectionChanged(SampleListBox *listBox, std::vector<int> &selection) override{};

    void selectionChanged(SampleListBox *listBox, int selection) override;

    void showActiveSample();

    const int fontSize{16};
    const int margin{8};
    const int btnSize{80};

    size_t activePad{0};

    mck::sampler::Config sampleConfig;
    std::vector<mck::SamplePack> samplePacks;

    juce::Label curSampleLabel;
    juce::TextButton showSampleButton;

    juce::TextButton autoPlayButton;

    juce::TextButton prevSampleButton;
    juce::TextButton nextSampleButton;
    juce::TextButton previewButton;
    juce::TextButton assignButton;

    SampleListBox packList{false};
    SampleListBox catList{false};
    SampleListBox sampleList{false};

    State state;
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

class PadsComponent : public juce::Component, public DrumPadComponent::Listener
{
public:
    PadsComponent()
    {
        auto conf = mck::Processing::GetInstance()->GetCurrentConfig();
        size_t i = 0;
        std::string title1 = "";
        std::string title2 = "";
        for (auto &p : pads)
        {
            title1 = "0" + std::to_string(i + 1);
            if (i < conf.numPads)
            {
                if (conf.pads[i].sampleId != "")
                {
                    title1 = title1 + " - " + conf.pads[i].sampleType;
                    title2 = conf.pads[i].sampleName;
                }
            }

            p.setPadText(title1 + "\n\n" + title2);
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
    void padDown(DrumPadComponent *d, double strength) override;

    void padUp(DrumPadComponent *d, double strength) override;


    const static size_t numPads{8};
    const static size_t numRows{2};
    const static size_t numCols{4};
    const static int margin{8};

    DrumPadComponent pads[numPads];
};