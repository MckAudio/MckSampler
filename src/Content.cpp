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
    curSampleLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(curSampleLabel);

    addAndMakeVisible(packList);
    addAndMakeVisible(catList);
    addAndMakeVisible(sampleList);

    previewButton.setButtonText("Preview");
    previewButton.onClick = [this](){ playSample(); };
    assignButton.setButtonText("Assign");
    assignButton.onClick = [this](){ assignSample(); };

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
    auto area = getLocalBounds();
    curSampleLabel.setBounds(area.removeFromTop(2 * margin + fontSize).reduced(margin));

    auto w3 = area.getWidth() / 3;
    area.removeFromBottom(80);

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
    // sampleConfig.pads[activePad].sampleName
    curSampleLabel.setText("Sample for Pad #" + std::to_string(activePad + 1), juce::NotificationType::dontSendNotification);

    std::vector<std::string> packs;
    for (auto &sp : samplePacks)
    {
        packs.push_back(sp.name);
    }

    packList.setItems(packs);
    updateCategories(packSelection);
    updateSamples(packSelection, catSelection);
}

void SampleComponent::updateCategories(std::vector<int> &packSel)
{
    std::set<std::string> cats;

    if (packSel.size() == 0)
    {
        for (auto &sp : samplePacks)
        {
            for (auto &c : sp.categories)
            {
                cats.emplace(c);
            }
        }
    }
    else
    {
        for (auto p : packSel)
        {
            for (auto &c : samplePacks[p].categories)
            {
                cats.emplace(c);
            }
        }
    }
    catNames.clear();
    catNames.reserve(cats.size());
    for (auto c : cats)
    {
        catNames.push_back(c);
    }

    catList.setItems(catNames);

    updateSamples(packSel, catSelection);
}

void SampleComponent::updateSamples(std::vector<int> &packs, std::vector<int> &cats)
{
    std::vector<std::string> samples;
    sampleMetas.clear();

    for (int i = 0; i < packs.size(); i++)
    {
        auto &sp = samplePacks[packs[i]];
        for (auto &c : cats)
        {
            auto res = std::find(sp.categories.begin(), sp.categories.end(), catNames[c]);
            if (res != sp.categories.end())
            {
                auto idx = std::distance(sp.categories.begin(), res);
                for (int j = 0; j < sp.samples.size(); j++)
                {
                    if (sp.samples[j].type == idx) {
                        samples.push_back(sp.samples[j].name);
                        sampleMetas.push_back(SampleMeta(j, packs[i]));
                    }
                }
            }
        }
    }

    sampleList.setItems(samples);
}

void SampleComponent::playSample()
{
    mck::SampleCommand cmd;
    cmd.packIdx = sampleMetas[activeSampleMeta].packIdx;
    cmd.sampleIdx = sampleMetas[activeSampleMeta].sampleIdx;
    cmd.padIdx = activePad;
    cmd.type = "play";
    mck::Processing::GetInstance()->SetSample(cmd);
}
void SampleComponent::assignSample()
{
    mck::SampleCommand cmd;
    cmd.packIdx = sampleMetas[activeSampleMeta].packIdx;
    cmd.sampleIdx = sampleMetas[activeSampleMeta].sampleIdx;
    cmd.padIdx = activePad;
    cmd.type = "assign";
    mck::Processing::GetInstance()->SetSample(cmd);
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
void SampleComponent::selectionChanged(SampleListBox *listBox, std::vector<int> &selection)
{
    if (listBox == &packList)
    {
        packSelection = selection;
        updateCategories(packSelection);
    }
    else if (listBox == &catList)
    {
        catSelection = selection;
        updateSamples(packSelection, catSelection);
    }
    else if (listBox == &sampleList)
    {
        if (selection.size() > 0)
        {
            //selectSample(selection[0]);
            activeSampleMeta = selection[0];
        }
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