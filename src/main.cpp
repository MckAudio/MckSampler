#include <stdio.h>  // fprintf
#include <unistd.h> // sleep
#include <stdlib.h>
#include <signal.h>
#include <string>
#include <string.h>
#include <iostream>
#include <vector>
#include <deque>

// Audio
#include <jack/jack.h>
#include <jack/midiport.h>
#include <sndfile.h>
#include <samplerate.h>

// Files & JSON
#include <sys/types.h>
#include <pwd.h>
#include <filesystem>
#include <fstream>
#include <nlohmann/json.hpp>

// Threading
#include <thread>
#include <atomic>
#include <condition_variable>
#include <mutex>

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


bool playSample = false;
bool resetSample = false;

unsigned configMode = 0;

unsigned configIdx = CONFIG_NONE;

float *wavBuffer;
float gain = 1.0;

mck::sampler::Config m_config;
std::filesystem::path m_configPath;
std::filesystem::path m_samplePath;

// Wav Files
std::vector<MCK::AudioSample> m_samples;
std::vector<MCK::AudioVoice> m_voices;
unsigned numVoices;
unsigned voiceIdx;

// Pad Trigger
std::deque<std::pair<unsigned, double>> m_trigger;
std::mutex m_triggerMutex;
std::atomic<bool> m_triggerActive = false;
std::condition_variable m_triggerCond;


void CloseApplication(bool saveConnections = true)
{


    using namespace std::filesystem;
    using namespace nlohmann;

    m_done = true;


    for (auto &s : m_samples)
    {
        if (s.buffer != nullptr)
        {
            delete s.buffer;
        }
    }

    std::ofstream configFile(m_configPath.string());
    json j = m_config;
    configFile << std::setw(4) << j << std::endl;
    configFile.close();

    m_processing.Close();
    m_gui.Close();
    exit(0);
}

static void SignalHandler(int sig)
{
    fprintf(stdout, "Signal %d received, exiting...\n", sig);
    CloseApplication();
}

int main(int argc, char **argv)
{
    using namespace std::filesystem;
    using namespace nlohmann;

    // Init Processing
    bool ret = m_processing.Init();
    if (ret == false) {
        return EXIT_FAILURE;
    }


    // Read Config
    struct passwd *pw = getpwuid(getuid());
    path configPath(pw->pw_dir);
    configPath.append(".mck").append("sampler");
    if (exists(configPath) == false)
    {
        create_directories(configPath);
    }
    m_samplePath = configPath;
    m_samplePath.append("audio");
    if (exists(m_samplePath) == false)
    {
        create_directories(m_samplePath);
    }
    configPath.append("config.json");
    bool createFile = true;
    if (exists(configPath))
    {
        std::ifstream configFile(configPath.string());
        json j;
        configFile >> j;
        configFile.close();
        try
        {
            m_config = j.get<mck::sampler::Config>();
            createFile = false;
        }
        catch (std::exception &e)
        {
            std::cout << "Failed to convert the config file: " << e.what() << std::endl;
        }
    }

    if (createFile)
    {
        std::ofstream configFile(configPath.string());
        json j = m_config;
        configFile << std::setw(4) << j << std::endl;
        configFile.close();
    }
    m_configPath = configPath;

    // Scan wav files
    mck::sampler::ScanSampleFolder(m_samplePath.string(), m_config.samples);
    m_config.numSamples = m_config.samples.size();

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
    }
    else if (m_config.numPads == 0)
    {
        m_config.pads.resize(16);
        mck::sampler::VerifyConfiguration(m_config);
    }

    mck::sampler::VerifyConfiguration(m_config);


    // Prepare Sound Files and Voices
    numVoices = 4 * m_config.numPads;
    voiceIdx = 0;

    m_samples.resize(m_config.numPads);
    m_voices.resize(numVoices);

    SNDFILE *tmpSnd;
    SF_INFO tmpInfo;
    SRC_DATA tmpSrc;
    float *tmpBuf;
    float *tmpSrcBuf;
    unsigned numSrcFrames;

    for (unsigned i = 0; i < m_config.numPads; i++)
    {
        if (m_config.pads[i].sampleIdx >= m_config.numSamples)
        {
            m_config.pads[i].available = false;
            continue;
        }
        m_config.pads[i].available = true;
        tmpInfo.format = 0;
        tmpSnd = sf_open(m_config.samples[m_config.pads[i].sampleIdx].fullPath.c_str(), SFM_READ, &tmpInfo);
        m_samples[i].numChans = tmpInfo.channels;
        m_samples[i].numFrames = tmpInfo.frames;
        tmpBuf = new float[tmpInfo.channels * tmpInfo.frames];
        numSrcFrames = sf_readf_float(tmpSnd, tmpBuf, tmpInfo.frames);
        m_samples[i].numFrames = numSrcFrames;
        if (tmpInfo.samplerate != sampleRate)
        {
            // Sample Rate Conversion
            double convCoeff = (double)sampleRate / (double)tmpInfo.samplerate;
            numSrcFrames = (unsigned)std::ceil((double)m_samples[i].numFrames * convCoeff);
            tmpSrcBuf = new float[m_samples[i].numChans * numSrcFrames];
            tmpSrc.data_in = tmpBuf;
            tmpSrc.data_out = tmpSrcBuf;
            tmpSrc.input_frames = m_samples[i].numFrames;
            tmpSrc.output_frames = numSrcFrames;
            numSrcFrames = tmpSrc.output_frames;
            tmpSrc.src_ratio = convCoeff;

            int err = src_simple(&tmpSrc, SRC_SINC_BEST_QUALITY, m_samples[i].numChans);
            if (err != 0)
            {
                fprintf(stderr, "Failed to apply samplerate conversion to sample %s:\n%s\nExiting...", m_config.pads[i].sample.c_str(), src_strerror(err));
                delete tmpBuf;
                delete tmpSrcBuf;
                CloseApplication(false);
            }
            m_samples[i].numFrames = tmpSrc.output_frames_gen;
            /*
            SNDFILE *outSnd;
            SF_INFO outInf;
            outInf.channels = m_samples[i].numChans;
            outInf.format = SF_FORMAT_WAV | SF_FORMAT_FLOAT;
            outInf.frames = m_samples[i].numFrames;
            outInf.samplerate = sampleRate;
            std::filesystem::path outPath(m_config.samples[m_config.pads[i].sampleIdx].fullPath);
            std::filesystem::path outName = outPath.filename();
            outName = std::filesystem::path(outName.stem().string() + "_" + std::to_string(outInf.samplerate) + outName.extension().string());
            outPath = outPath.parent_path().append(outName.string());
            outSnd = sf_open(outPath.c_str(), SFM_WRITE, &outInf);
            sf_writef_float(outSnd, tmpSrcBuf, m_samples[i].numFrames);
            sf_close(outSnd);
            */
            delete tmpBuf;
            tmpBuf = tmpSrcBuf;
        }
        // Deinterleaving

        m_samples[i].buffer = new float[m_samples[i].numChans * m_samples[i].numFrames];
        /*
        m_samples[i].pitchBuffer = new float *[m_samples[i].numChans];
        m_samples[i].outBuffer = new float *[m_samples[i].numChans];
        for (unsigned j = 0; j < m_samples[i].numChans; j++)
        {
            m_samples[i].pitchBuffer[j] = new float[bufferSize];
            m_samples[i].outBuffer[j] = new float[bufferSize];
        }*/
        for (unsigned s = 0; s < std::min(m_samples[i].numFrames, numSrcFrames); s++)
        {
            for (unsigned c = 0; c < m_samples[i].numChans; c++)
            {
                m_samples[i].buffer[c * m_samples[i].numFrames + s] = tmpBuf[s * m_samples[i].numChans + c];
            }
        }
        delete tmpBuf;

        // init pitcher
        //m_samples[i].pitcher = new RubberBand::RubberBandStretcher(sampleRate, m_samples[i].numChans, RubberBand::RubberBandStretcher::OptionProcessRealTime, 1.0, 1.0);
        //m_samples[i].pitcher->setMaxProcessSize(bufferSize);
    }

    signal(SIGQUIT, SignalHandler);
    signal(SIGTERM, SignalHandler);
    signal(SIGHUP, SignalHandler);
    signal(SIGINT, SignalHandler);

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
    }

    m_gui.Show("MckSampler", "./www", 9002);

    //sleep(-1);

    //guiWindow.run();

    CloseApplication();

    return EXIT_SUCCESS;
}
