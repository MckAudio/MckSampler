#include "Content.hpp"

//>  ControlPageComponent  //
ControlPageComponent::ControlPageComponent()
{
    openButton.setButtonText("Open Sample");
    openButton.onClick = [this]
    { openButtonClicked(); };
    addAndMakeVisible(openButton);

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
    auto area = getLocalBounds();
    openButton.setBounds(area.reduced(8));
}
void ControlPageComponent::configChanged(const mck::sampler::Config &config)
{
    std::printf("Active pad #%d\n", config.activePad + 1);
    activePad = config.activePad;
}

void ControlPageComponent::sliderValueChanged(Slider *slider)
{
}

void ControlPageComponent::openButtonClicked()
{
    fileChooser = std::make_unique<juce::FileChooser>("Select a sample...", juce::File{}, "*.wav");
    auto chooserFlags = juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles;

    fileChooser->launchAsync(chooserFlags, [this](const FileChooser &fc)
                             {
        auto res = fc.getResult();
        std::cout << res.getFullPathName() << std::endl; });
}

//  ControlPageComponent <//

//> SampleComponent  //

SampleComponent::SampleComponent()
{
    packList.addListener(this);
    catList.addListener(this);
    sampleList.addListener(this);

    curSampleLabel.setFont(juce::Font(fontSize, juce::Font::plain));
    curSampleLabel.setJustificationType(juce::Justification::centredLeft);
    addAndMakeVisible(curSampleLabel);
    showSampleButton.setButtonText("Show");
    showSampleButton.setEnabled(false);
    showSampleButton.onClick = [this]()
    { showActiveSample(); };
    addAndMakeVisible(showSampleButton);

    addAndMakeVisible(packList);
    addAndMakeVisible(catList);
    addAndMakeVisible(sampleList);

    autoPlayButton.setButtonText("Auto Play");
    autoPlayButton.setEnabled(false);
    autoPlayButton.setToggleable(true);
    addAndMakeVisible(autoPlayButton);
    autoPlayButton.onClick = [this]()
    { state.autoPlay = !state.autoPlay;
        updateControls(); };

    prevSampleButton.setButtonText("<");
    prevSampleButton.setEnabled(false);
    prevSampleButton.onClick = [this]()
    { previousSample(); };
    nextSampleButton.setButtonText(">");
    nextSampleButton.setEnabled(false);
    nextSampleButton.onClick = [this]()
    { nextSample(); };
    previewButton.setButtonText("Play");
    previewButton.setEnabled(false);
    previewButton.onClick = [this]()
    { playSample(); };
    assignButton.setButtonText("Assign");
    assignButton.setEnabled(false);
    assignButton.onClick = [this]()
    { assignSample(); };

    addAndMakeVisible(prevSampleButton);
    addAndMakeVisible(nextSampleButton);
    addAndMakeVisible(previewButton);
    addAndMakeVisible(assignButton);

    mck::Processing::GetInstance()->addListener(this);
}

SampleComponent::~SampleComponent()
{
    packList.removeListener(this);
    catList.removeListener(this);
    sampleList.removeListener(this);

    mck::Processing::GetInstance()->removeListener(this);
}

void SampleComponent::paint(juce::Graphics &g)
{
}

void SampleComponent::resized()
{
    auto topArea = getLocalBounds();
    auto w3 = topArea.getWidth() / 3;

    topArea.removeFromBottom(topArea.getHeight() - (btnSize / 2));
    curSampleLabel.setBounds(topArea.removeFromLeft(2 * w3).reduced(margin));
    showSampleButton.setBounds(topArea.reduced(margin));

    auto area = getLocalBounds();
    area.removeFromTop(btnSize / 2);
    area.removeFromBottom(btnSize);

    packList.setBounds(area.removeFromLeft(w3).reduced(margin));
    catList.setBounds(area.removeFromLeft(w3).reduced(margin));
    sampleList.setBounds(area.removeFromLeft(w3).reduced(margin));

    auto bottomArea = getLocalBounds();
    bottomArea.removeFromTop(bottomArea.getHeight() - btnSize);

    auto btmLeftArea = bottomArea;
    autoPlayButton.setBounds(btmLeftArea.removeFromLeft(btnSize).reduced(margin));

    assignButton.setBounds(bottomArea.removeFromRight(btnSize).reduced(margin));
    previewButton.setBounds(bottomArea.removeFromRight(btnSize).reduced(margin));
    nextSampleButton.setBounds(bottomArea.removeFromRight(btnSize).reduced(margin));
    prevSampleButton.setBounds(bottomArea.removeFromRight(btnSize).reduced(margin));
}
SampleComponent::State SampleComponent::getState()
{
    state.init = true;
    return state;
}
void SampleComponent::setState(State &newState)
{
    if (newState.init)
    {
        state = newState;
        packList.setItems(state.packNames);
        catList.setItems(state.catNames);
        sampleList.setItems(state.sampleNames);
        packList.setActiveRow(state.activePackIdx);
        catList.setActiveRow(state.activeCatIdx);
        sampleList.setActiveRow(state.activeSampleIdx);
        updateControls();
    }
}

void SampleComponent::update()
{
    std::vector<std::string> names;
    std::string tmp = "Sample for Pad #" + std::to_string(activePad + 1);
    if (sampleConfig.pads[activePad].sampleName != "")
    {
        tmp = tmp + ":\t" + sampleConfig.pads[activePad].sampleName;
        showSampleButton.setEnabled(true);
    }
    else
    {
        showSampleButton.setEnabled(false);
    }
    curSampleLabel.setText(tmp, juce::NotificationType::dontSendNotification);

    int activeIdx = -1;
    names.resize(samplePacks.size());
    for (size_t i = 0; i < samplePacks.size(); i++)
    {
        names[i] = samplePacks[i].name;
        activeIdx = names[i] == state.activePackName ? i : activeIdx;
    }
    if (activeIdx < 0)
    {
        state.activePackName = "";
    }
    state.activePackIdx = activeIdx;

    if (names != state.packNames)
    {
        state.packNames = names;
        packList.setItems(state.packNames);
        packList.setActiveRow(activeIdx);
    }

    updateCategories(state.activePackIdx);
}

void SampleComponent::updateControls()
{
    prevSampleButton.setEnabled(state.activeSampleIdx > 0 && state.activeSampleIdx < state.sampleNames.size());
    nextSampleButton.setEnabled(state.activeSampleIdx >= 0 && state.activeSampleIdx < state.sampleNames.size() - 1);
    assignButton.setEnabled(state.activeSampleIdx >= 0 && state.activeSampleIdx < state.sampleNames.size());
    previewButton.setEnabled(state.activeSampleIdx >= 0 && state.activeSampleIdx < state.sampleNames.size());
    autoPlayButton.setEnabled(state.activeSampleIdx >= 0 && state.activeSampleIdx < state.sampleNames.size());
    autoPlayButton.setToggleState(state.autoPlay, false);
}

void SampleComponent::updateCategories(int activePack)
{
    std::vector<std::string> names;

    if (activePack < 0 || activePack >= samplePacks.size())
    {
        state.activePackName = "";
        state.activePackIdx = -1;
        catList.setItems(names);
        catList.setActiveRow(-1);
        return;
    }

    state.activePackName = samplePacks[activePack].name;
    state.activePackIdx = activePack;

    names.clear();
    names.insert(names.begin(), samplePacks[activePack].categories.begin(), samplePacks[activePack].categories.end());

    if (names != state.catNames)
    {
        state.catNames = names;

        // state.activePackName = state.packNames[activePack]
        int catIdx = -1;
        auto it = std::find(state.catNames.begin(), state.catNames.end(), state.activeCatName);
        if (it != state.catNames.end())
        {
            catIdx = std::distance(state.catNames.begin(), it);
        }

        catList.setItems(state.catNames);
        catList.setActiveRow(catIdx);

        if (catIdx > 0)
        {
            state.activeCatName = state.catNames[catIdx];
            state.activeCatIdx = catIdx;
        }
        else
        {
            state.activeCatName = "";
            state.activeCatIdx = -1;
        }
    }

    updateSamples(state.activePackIdx, state.activeCatIdx);
}

void SampleComponent::updateSamples(int activePack, int activeCat)
{
    std::vector<std::string> samples;
    std::vector<SampleMeta> metas;

    if (activePack < 0 || activePack >= samplePacks.size())
    {
        state.activePackIdx = -1;
        state.activePackName = "";
        return;
    }
    else if (activeCat < 0 || activeCat >= samplePacks[activePack].categories.size())
    {
        state.activeCatIdx = -1;
        state.activeCatName = "";
        sampleList.setItems(samples);
        sampleList.setActiveRow(-1);
        return;
    }

    state.activeCatIdx = activeCat;
    state.activeCatName = samplePacks[activePack].categories[activeCat];

    size_t sIdx = 0;
    for (auto &s : samplePacks[activePack].samples)
    {
        if (s.type == activeCat)
        {
            samples.push_back(s.name);
            metas.push_back(SampleMeta(sIdx, activePack));
        }
        sIdx++;
    }

    if (samples != state.sampleNames)
    {
        state.sampleNames = samples;
        state.sampleMetas = metas;

        int sampleIdx = -1;
        auto it = std::find(state.sampleNames.begin(), state.sampleNames.end(), state.activeSampleName);
        if (it != state.sampleNames.end())
        {
            sampleIdx = std::distance(state.sampleNames.begin(), it);
        }

        sampleList.setItems(state.sampleNames);
        sampleList.setActiveRow(sampleIdx);

        if (sampleIdx > 0)
        {
            state.activeSampleName = state.sampleNames[sampleIdx];
            state.activeSampleIdx = sampleIdx;
        }
        else
        {
            state.activeSampleName = "";
            state.activeSampleIdx = -1;
        }
    }

    updateControls();
}

void SampleComponent::previousSample()
{
    if (state.activeSampleIdx > 0)
    {
        state.activeSampleIdx -= 1;
        sampleList.setActiveRow(state.activeSampleIdx);
        if (state.autoPlay)
        {
            playSample();
        }
    }
}
void SampleComponent::nextSample()
{
    if (state.activeSampleIdx >= 0 && state.activeSampleIdx < state.sampleNames.size() - 1)
    {
        state.activeSampleIdx += 1;
        sampleList.setActiveRow(state.activeSampleIdx);
        if (state.autoPlay)
        {
            playSample();
        }
    }
}
void SampleComponent::playSample()
{
    if (state.activeSampleIdx >= 0)
    {
        mck::SampleCommand cmd;
        cmd.packIdx = state.sampleMetas[state.activeSampleIdx].packIdx;
        cmd.sampleIdx = state.sampleMetas[state.activeSampleIdx].sampleIdx;
        cmd.padIdx = activePad;
        cmd.type = "play";
        mck::Processing::GetInstance()->SetSample(cmd);
    }
}
void SampleComponent::assignSample()
{
    if (state.activeSampleIdx >= 0)
    {
        mck::SampleCommand cmd;
        cmd.packIdx = state.sampleMetas[state.activeSampleIdx].packIdx;
        cmd.sampleIdx = state.sampleMetas[state.activeSampleIdx].sampleIdx;
        cmd.padIdx = activePad;
        cmd.type = "assign";
        mck::Processing::GetInstance()->SetSample(cmd);
    }
}

void SampleComponent::configChanged(const mck::sampler::Config &config)
{
    activePad = config.activePad;
    sampleConfig = config;
    update();
}

void SampleComponent::samplesChanged(const std::vector<mck::SamplePack> &samples)
{
    samplePacks = samples;
    update();
}

void SampleComponent::selectionChanged(SampleListBox *listBox, int selection)
{
    if (listBox == &packList)
    {
        updateCategories(selection);
    }
    else if (listBox == &catList)
    {
        updateSamples(state.activePackIdx, selection);
    }
    else if (listBox == &sampleList)
    {
        state.activeSampleIdx = selection;
        updateControls();

        if (state.autoPlay)
        {
            playSample();
        }
    }
}

void SampleComponent::showActiveSample()
{
    const auto &sampleId = sampleConfig.pads[sampleConfig.activePad].sampleId;

    if (sampleId != "")
    {
        size_t spIdx = 0;
        for (auto &sp : samplePacks)
        {
            for (auto &s : sp.samples)
            {
                if (s.id == sampleId)
                {
                    state.activePackIdx = spIdx;
                    updateCategories(state.activePackIdx);
                    state.activeCatIdx = s.type;
                    updateSamples(state.activePackIdx, state.activeCatIdx);
                    state.activeSampleIdx = s.index - 1;
                    packList.setActiveRow(state.activePackIdx);
                    catList.setActiveRow(state.activeCatIdx);
                    sampleList.setActiveRow(state.activeSampleIdx);
                    updateControls();
                    return;
                }
            }
            spIdx++;
        }
    }
    else
    {
        showSampleButton.setEnabled(false);
    }
}

//  SampleComponent <//

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