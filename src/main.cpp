#include <cstdio>  // fprintf
#include <unistd.h> // sleep
#include <stdlib.h>
#include <signal.h>
#include <string>
#include <string.h>
#include <iostream>

// GUI
#include "GuiWindow.hpp"

// OWN Header
#include "Config.hpp"
#include "helper/JackHelper.hpp"
#include "helper/Transport.hpp"
#include "Processing.hpp"

enum
{
    CONFIG_NONE = 0,
    CONFIG_SAMPLES,
    CONFIG_PADS,
    CONFIG_CTRLS,
    CONFIG_MODES,
    CONFIG_LENGTH
};

// APP
std::atomic<bool> m_done = false;

// GUI
GuiWindow m_gui;

// PROCESSING
mck::Processing m_processing;

// FILES
std::string wavFile = "../content/risset_long.wav";
SNDFILE *sndFile;
SF_INFO sndInfo;

unsigned configMode = 0;

unsigned configIdx = CONFIG_NONE;

mck::sampler::Config m_config;
std::filesystem::path m_configPath;
std::filesystem::path m_samplePath;


void CloseApplication(bool saveConnections = true)
{
    using namespace std::filesystem;
    using namespace nlohmann;

    m_done = true;

    m_processing.Close();
    m_gui.Close();
    exit(0);
}

static void SignalHandler(int sig)
{
    std::fprintf(stdout, "Signal %d received, exiting...\n", sig);
    CloseApplication();
}

int main(int argc, char **argv)
{
    // Get Arguments
    if (argc >= 2)
    {
        std::string command = std::string(argv[1]);
        if (command == "--config" || command == "-c")
        {
            configMode = CONFIG_SAMPLES;
            printf("MIDI SamplePad Sample Configuration Mode:\n");
        }
        else if (command == "--pads" || command == "-p")
        {
            configMode = CONFIG_PADS;
            printf("MIDI SamplePad Pad Configuration Mode:\n");
        }
        else if (command == "--control" || command == "-C")
        {
            configMode = CONFIG_CTRLS;
            printf("MIDI SamplePad Control Configuration Mode:\n");
        }
        else if (command == "--help" || command == "-h")
        {
            printf("MIDI SamplePad by Matt McK\nUsage:\n\tmcksampler [OPTION]\nOptions:\n");
            printf("\t--config, -c\tConfigure number of pads and sample per pad\n");
            printf("\t--pads, -p\tConfigure tone per pad\n");
            printf("\t--control, -C\tConfigure control per pad\n");
            printf("\t--help, -h\tShow this help\n");
            exit(0);
        }
    }

    // Init Processing
    bool ret = m_processing.Init();
    if (ret == false) {
        return EXIT_FAILURE;
    }

    /*
    if (configMode == CONFIG_SAMPLES)
    {
        printf("Midi Channel? (%d):\n", m_config.midiChan + 1);
        int midiChan = m_config.midiChan + 1;
        std::cin >> midiChan;
        midiChan = std::max(1, std::min(16, midiChan));
        m_config.midiChan = midiChan - 1;

        printf("Number of pads? (%d):\n", m_config.numPads);
        unsigned numPads = m_config.numPads;
        std::cin >> numPads;
        m_config.numPads = std::max((unsigned)0, std::min((unsigned)64, numPads));
        m_config.pads.resize(m_config.numPads);
        printf("\nThese are your samples:\n");
        for (unsigned j = 0; j < m_config.numSamples; j++)
        {
            printf("\t[%d]\t%s\n", j + 1, m_config.samples[j].name.c_str());
        }
        for (unsigned i = 0; i < m_config.numPads; i++)
        {
            printf("\nPlease select the sample for pad #%d: ", i + 1);
            int sampleIdx = 1;
            std::cin >> sampleIdx;
            std::cout << std::endl;
            sampleIdx = std::max(1, std::min((int)m_config.numSamples, sampleIdx)) - 1;
            m_config.pads[i].sample = m_config.samples[sampleIdx].relativePath;
            m_config.pads[i].sampleIdx = sampleIdx;
        }
    }*/

    signal(SIGQUIT, SignalHandler);
    signal(SIGTERM, SignalHandler);
    signal(SIGHUP, SignalHandler);
    signal(SIGINT, SignalHandler);

    /*
    if (m_config.numPads > 0)
    {
        if (configMode == CONFIG_PADS)
        {
            printf("Please play note for pad #%d on MIDI channel %d:\n", configIdx + 1, m_config.midiChan + 1);
        }
        else if (configMode == CONFIG_CTRLS)
        {
            printf("Please turn the controller for pad #%d on MIDI channel %d:\n", configIdx + 1, m_config.midiChan + 1);
        }
    }
    else
    {
        CloseApplication();
    }*/

    m_gui.Show("MckSampler", "./www", 9002);

    //sleep(-1);

    //guiWindow.run();

    CloseApplication();

    return EXIT_SUCCESS;
}
