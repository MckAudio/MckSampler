#include "MainComponent.hpp"
#include "Processing.hpp"

MainComponent::MainComponent()
{
    setSize(defaultWidth, defaultHeight);

    menuComponent.addListener(this);

    addAndMakeVisible(ctrlComponent);
    addAndMakeVisible(menuComponent);
    addAndMakeVisible(padComponent);
    addAndMakeVisible(selComponent);

    setActiveContent(Content::Mixer);

    resized();

    if (juce::RuntimePermissions::isRequired(juce::RuntimePermissions::recordAudio) && !juce::RuntimePermissions::isGranted(juce::RuntimePermissions::recordAudio))
    {
        juce::RuntimePermissions::request(juce::RuntimePermissions::recordAudio,
                                          [&](bool granted)
                                          { setAudioChannels(granted ? 2 : 0, 2); });
    }
    else
    {
        setAudioChannels(2, 2);
    }
}

MainComponent::~MainComponent()
{
    shutdownAudio();

    if (contentComponent != nullptr)
    {
        delete contentComponent;
        contentComponent = nullptr;
    }
}

void MainComponent::prepareToPlay(int samplePerBlockExpected, double sampleRate)
{
    mck::Processing::GetInstance()->Init(sampleRate, samplePerBlockExpected);
}
void MainComponent::getNextAudioBlock(const juce::AudioSourceChannelInfo &bufferToFill)
{
    bufferToFill.clearActiveBufferRegion();
    if (bufferToFill.buffer->getNumChannels() >= 2) {
        float *outL = bufferToFill.buffer->getWritePointer(0, bufferToFill.startSample);
        float *outR = bufferToFill.buffer->getWritePointer(1, bufferToFill.startSample);
        mck::Processing::GetInstance()->Process(outL, outR, bufferToFill.numSamples);
    }
}
void MainComponent::releaseResources()
{
    mck::Processing::GetInstance()->Close();
}