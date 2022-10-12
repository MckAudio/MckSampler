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

    previewButton.setButtonText("Preview");
    previewButton.setEnabled(false);
    previewButton.onClick = [this]()
    { playSample(); };
    assignButton.setButtonText("Assign");
    assignButton.setEnabled(false);
    assignButton.onClick = [this]()
    { assignSample(); };

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

    assignButton.setBounds(bottomArea.removeFromRight(btnSize).reduced(margin));
    previewButton.setBounds(bottomArea.removeFromRight(btnSize).reduced(margin));
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
        activeIdx = names[i] == activePackName ? i : activeIdx;
    }
    if (activeIdx < 0)
    {
        activePackName = "";
    }
    activePackIdx = activeIdx;

    if (names != packNames) {
        packNames = names;
        packList.setItems(packNames);
        packList.setActiveRow(activeIdx);
    }

    updateCategories(activePackIdx);
}

void SampleComponent::updateControls()
{
    assignButton.setEnabled(activeSampleMeta >= 0 && activeSampleMeta < sampleMetas.size());
    previewButton.setEnabled(activeSampleMeta >= 0 && activeSampleMeta < sampleMetas.size());
}

void SampleComponent::updateCategories(int activePack)
{
    std::vector<std::string> names;

    if (activePack < 0 || activePack >= samplePacks.size())
    {
        activePackName = "";
        activePackIdx = -1;
        catList.setItems(names);
        catList.setActiveRow(-1);
        return;
    }

    activePackName = samplePacks[activePack].name;
    activePackIdx = activePack;

    names.clear();
    names.insert(names.begin(), samplePacks[activePack].categories.begin(), samplePacks[activePack].categories.end());

    if (names != catNames)
    {
        catNames = names;

        // activePackName = packNames[activePack]
        int catIdx = -1;
        auto it = std::find(catNames.begin(), catNames.end(), activeCatName);
        if (it != catNames.end())
        {
            catIdx = std::distance(catNames.begin(), it);
        }

        catList.setItems(catNames);
        catList.setActiveRow(catIdx);
        
        if (catIdx > 0) {
            activeCatName = catNames[catIdx];
            activeCatIdx = catIdx;
        } else {
            activeCatName = "";
            activeCatIdx = -1;
        }
    }

    updateSamples(activePackIdx, activeCatIdx);
}

void SampleComponent::updateSamples(int activePack, int activeCat)
{
    std::vector<std::string> samples;

    if (activePack < 0 || activePack >= samplePacks.size())
    {
        activePackIdx = -1;
        activePackName = "";
        return;
    }
    else if (activeCat < 0 || activeCat >= samplePacks[activePack].categories.size())
    {
        activeCatIdx = -1;
        activeCatName = "";
        sampleList.setItems(samples);
        sampleList.setActiveRow(-1);
        return;
    }

    sampleMetas.clear();

    size_t sIdx = 0;
    for (auto &s : samplePacks[activePack].samples)
    {
        if (s.type == activeCat)
        {
            samples.push_back(s.name);
            sampleMetas.push_back(SampleMeta(sIdx, activePack));
        }
        sIdx++;
    }

    sampleList.setItems(samples);

    activeCatIdx = activeCat;
    activeCatName = samplePacks[activePack].categories[activeCat];
    catList.setActiveRow(activeCatIdx);

    updateControls();
}

void SampleComponent::playSample()
{
    if (activeSampleMeta >= 0)
    {
        mck::SampleCommand cmd;
        cmd.packIdx = sampleMetas[activeSampleMeta].packIdx;
        cmd.sampleIdx = sampleMetas[activeSampleMeta].sampleIdx;
        cmd.padIdx = activePad;
        cmd.type = "play";
        mck::Processing::GetInstance()->SetSample(cmd);
    }
}
void SampleComponent::assignSample()
{
    if (activeSampleMeta >= 0)
    {
        mck::SampleCommand cmd;
        cmd.packIdx = sampleMetas[activeSampleMeta].packIdx;
        cmd.sampleIdx = sampleMetas[activeSampleMeta].sampleIdx;
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
        updateSamples(activePackIdx, selection);
    }
    else if (listBox == &sampleList)
    {
        activeSampleMeta = selection;
        updateControls();
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
                    activePackIdx = spIdx;
                    activeCatIdx = s.type;
                    updateCategories(activePackIdx);
                    packList.setActiveRow(activePackIdx);
                    catList.setActiveRow(activeCatIdx);
                    sampleList.setActiveRow(s.index - 1);
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