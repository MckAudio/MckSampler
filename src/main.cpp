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
#include "../webview/webview.h"
#include "../cpp-httplib/httplib.h"

// OWN Header
#include "Config.hpp"
#include "JackHelper.hpp"

enum
{
    CONFIG_NONE = 0,
    CONFIG_SAMPLES,
    CONFIG_PADS,
    CONFIG_CTRLS,
    CONFIG_MODES,
    CONFIG_LENGTH
};

// GUI
httplib::Server guiServer;
std::thread guiServerThread;
webview::webview guiWindow(true, nullptr);
std::thread guiWindowThread;

// FILES
std::string wavFile = "../content/risset_long.wav";
SNDFILE *sndFile;
SF_INFO sndInfo;

// JACK
jack_client_t *client;
jack_port_t *midi_in;
jack_port_t *audio_out_l;
jack_port_t *audio_out_r;

jack_nframes_t bufferSize;
jack_nframes_t sampleRate;

bool playSample = false;
bool resetSample = false;

unsigned configMode = 0;

unsigned configIdx = CONFIG_NONE;

float *wavBuffer;
float gain = 1.0;

SP::Config m_config;
std::filesystem::path m_configPath;
std::filesystem::path m_samplePath;

// Wav Files
std::vector<MCK::AudioSample> m_samples;
std::vector<MCK::AudioVoice> m_voices;
unsigned numVoices;
unsigned voiceIdx;

// Pad Trigger
std::deque<unsigned> m_trigger;
std::mutex m_triggerMutex;
std::atomic<bool> m_triggerActive = false;
std::condition_variable m_triggerCond;

void CloseApplication(bool saveConnections = true)
{
    using namespace std::filesystem;
    using namespace nlohmann;

    if (client != nullptr)
    {
        // Save Connections
        if (saveConnections)
        {
            MCK::JACK::GetConnections(client, midi_in, m_config.midiConnections);
            MCK::JACK::GetConnections(client, audio_out_l, m_config.audioLeftConnections);
            MCK::JACK::GetConnections(client, audio_out_r, m_config.audioRightConnections);
        }
        jack_client_close(client);
    }

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

    if (guiWindowThread.joinable())
    {
        guiWindow.terminate();
        guiWindowThread.join();
    }
    if (guiServerThread.joinable())
    {
        guiServer.stop();
        guiServerThread.join();
    }
    exit(0);
}

static void SignalHandler(int sig)
{
    fprintf(stdout, "Signal %d received, exiting...\n", sig);
    CloseApplication();
}

static int AudioProcess(jack_nframes_t nframes, void *arg)
{
    void *midi_buf = jack_port_get_buffer(midi_in, nframes);

    jack_nframes_t midiEventCount = jack_midi_get_event_count(midi_buf);
    jack_midi_event_t midiEvent;

    bool sysMsg = false;
    unsigned char chan = 0;

    for (unsigned i = 0; i < midiEventCount; i++)
    {
        jack_midi_event_get(&midiEvent, midi_buf, i);
        sysMsg = (midiEvent.buffer[0] & 0xf0) == 0xf0;
        chan = (midiEvent.buffer[0] & 0x0f);

        if (sysMsg == false && chan == m_config.midiChan)
        {
            if (configMode == CONFIG_PADS)
            {
                if ((midiEvent.buffer[0] & 0xf0) == 0x90)
                {
                    m_config.pads[configIdx].tone = (midiEvent.buffer[1] & 0x7f);
                    printf("Saved note %X for pad #%d.\n\n", m_config.pads[configIdx].tone, configIdx + 1);
                    configIdx += 1;
                    if (configIdx >= m_config.numPads)
                    {
                        printf("Finished configuration, entering play mode...\n");
                        configMode = false;
                        break;
                    }
                    printf("Please play note for pad #%d on MIDI channel %d:\n", configIdx + 1, m_config.midiChan + 1);
                    break;
                }
            }
            else if (configMode == CONFIG_CTRLS)
            {
                if ((midiEvent.buffer[0] & 0xf0) == 0xb0)
                {

                    if (configIdx > 0 && (midiEvent.buffer[1] & 0x7f) == m_config.pads[configIdx - 1].ctrl)
                    {
                        continue;
                    }
                    m_config.pads[configIdx].ctrl = (midiEvent.buffer[1] & 0x7f);
                    printf("Saved control %X for pad #%d.\n\n", m_config.pads[configIdx].ctrl, configIdx + 1);
                    configIdx += 1;
                    if (configIdx >= m_config.numPads)
                    {
                        printf("Finished configuration, entering play mode...\n");
                        configMode = CONFIG_NONE;
                        break;
                    }
                    printf("Please turn the controller for pad #%d on MIDI channel %d:\n", configIdx + 1, m_config.midiChan + 1);
                    break;
                }
            }
            else
            {
                if ((midiEvent.buffer[0] & 0xf0) == 0x90)
                {
                    for (unsigned j = 0; j < m_config.numPads; j++)
                    {
                        if ((midiEvent.buffer[1] & 0x7f) == m_config.pads[j].tone && m_config.pads[j].available)
                        {
                            m_voices[voiceIdx].playSample = true;
                            m_voices[voiceIdx].startIdx = midiEvent.time;
                            m_voices[voiceIdx].bufferIdx = 0;
                            m_voices[voiceIdx].gain = ((float)(midiEvent.buffer[2] & 0x7f) / 127.0f) * m_config.pads[j].gain;
                            m_voices[voiceIdx].sampleIdx = j;
                            m_voices[voiceIdx].pitch = m_config.pads[j].pitch;

                            voiceIdx = (voiceIdx + 1) % numVoices;
                        }
                    }
                }
                else if ((midiEvent.buffer[0] & 0xf0) == 0xb0)
                {
                    for (unsigned j = 0; j < m_config.numPads; j++)
                    {
                        if ((midiEvent.buffer[1] & 0x7f) == m_config.pads[j].ctrl)
                        {
                            m_config.pads[j].gain = (float)(midiEvent.buffer[2] & 0x7f) / 127.0f;
                            //m_config.pads[j].pitch = ((float)(midiEvent.buffer[2] & 0x7f) / 127.0f) * 1.5f + 0.5;
                        }
                    }
                }
            }
        }
    }

    m_triggerActive = true;
    while (m_trigger.size() > 0)
    {
        unsigned idx = m_trigger[0];
        m_trigger.pop_front();

        if (idx < m_config.numPads)
        {
            if (m_config.pads[idx].available)
            {
                m_voices[voiceIdx].playSample = true;
                m_voices[voiceIdx].startIdx = 0;
                m_voices[voiceIdx].bufferIdx = 0;
                m_voices[voiceIdx].gain = m_config.pads[idx].gain;
                m_voices[voiceIdx].sampleIdx = idx;
                m_voices[voiceIdx].pitch = m_config.pads[idx].pitch;

                voiceIdx = (voiceIdx + 1) % numVoices;
            }
        }
    }
    m_triggerActive = false;
    m_triggerCond.notify_all();

    if (resetSample)
    {
        sf_seek(sndFile, 0, SEEK_SET);
        resetSample = false;
    }

    jack_default_audio_sample_t *out_l = (jack_default_audio_sample_t *)jack_port_get_buffer(audio_out_l, nframes);
    jack_default_audio_sample_t *out_r = (jack_default_audio_sample_t *)jack_port_get_buffer(audio_out_r, nframes);

    memset(out_l, 0, nframes * sizeof(jack_default_audio_sample_t));
    memset(out_r, 0, nframes * sizeof(jack_default_audio_sample_t));

    unsigned len = 0;
    for (auto &v : m_voices)
    {
        if (v.playSample == false)
        {
            continue;
        }

        MCK::AudioSample *s = &m_samples[v.sampleIdx];
        /*
        for (unsigned i = 0; i < s->numChans; i++)
        {
            memset(s->pitchBuffer[i], 0, bufferSize * sizeof(float));
        }*/
        len = std::min(bufferSize, s->numFrames - v.bufferIdx) - v.startIdx;

        if (s->numChans > 1)
        {
            for (unsigned i = 0; i < len; i++)
            {
                out_l[v.startIdx + i] += s->buffer[v.bufferIdx + i] * v.gain;
                out_r[v.startIdx + i] += s->buffer[s->numFrames + v.bufferIdx + i] * v.gain;
            }
        }
        else
        {
            for (unsigned i = 0; i < len; i++)
            {
                out_l[v.startIdx + i] += s->buffer[v.bufferIdx + i] * v.gain * 0.707f;
                out_r[v.startIdx + i] += s->buffer[v.bufferIdx + i] * v.gain * 0.707f;
            }

            /*
            memcpy(s->pitchBuffer[0] + v.startIdx, s->buffer + v.bufferIdx, len);
            s->pitcher->setPitchScale(v.pitch);
            unsigned processed = 0;
            unsigned procOut = 0;
            while (processed < bufferSize)
            {
                unsigned procIn = s->pitcher->getSamplesRequired();
                procIn = std::min(bufferSize - processed, procIn);
                s->pitcher->process(s->pitchBuffer + processed, procIn, false);
                processed += procIn;
                int avail = s->pitcher->available();
                int actual = s->pitcher->retrieve(s->outBuffer + procOut, avail);
                procOut += actual;
            }

            for (unsigned i = 0; i < len; i++)
            {
                out_l[v.startIdx + i] += s->outBuffer[0][i] * v.gain * 0.707f;
                out_r[v.startIdx + i] += s->outBuffer[0][i] * v.gain * 0.707f;
            }
            */
        }
        v.bufferIdx += len;
        v.startIdx = 0;

        if (v.bufferIdx >= s->numFrames)
        {
            // Stop Sample
            v.playSample = false;
        }
    }

    return 0;
}

void MsgToFrontEnd(MCK::Message &msg)
{
    nlohmann::json j = msg;
    std::string out = "ReceiveMessage(\"" + j.dump() + "\");";
    std::cout << "Out: " << out << std::endl;
    guiWindow.eval(out);
}

std::string GetData(std::string msg)
{
    nlohmann::json jOut = m_config;
    return jOut.dump();
}

std::string SendMessage(std::string msg)
{
    using json = nlohmann::json;
    std::vector<MCK::Message> messages;
    try
    {
        json j = json::parse(msg);
        messages = j.get<std::vector<MCK::Message>>();
    }
    catch (std::exception &e)
    {
        std::cerr << "Failed to convert incoming message: " << e.what() << std::endl;
        return e.what();
    }
    std::cout << msg << std::endl;

    for (unsigned i = 0; i < messages.size(); i++)
    {
        if (messages[i].section == "pads")
        {
            if (messages[i].msgType == "trigger")
            {
                std::unique_lock<std::mutex> lock(m_triggerMutex);
                while (m_triggerActive.load())
                {
                    m_triggerCond.wait(lock);
                }
                MCK::TriggerData data = json::parse(messages[i].data);
                int triggerIdx = data.index;
                if (triggerIdx >= 0)
                {
                    std::cout << "Triggering PAD #" << (triggerIdx + 1) << std::endl;
                    m_trigger.push_back(triggerIdx);
                }
            }
        }
        else if (messages[i].section == "data")
        {
            if (messages[i].msgType == "get")
            {
                auto outMsg = MCK::Message();
                outMsg.section = "data";
                outMsg.msgType = "full";
                json jOut = m_config;
                outMsg.data = jOut.dump();
                MsgToFrontEnd(outMsg);
            }
        }
    }

    return "";
}

int main(int argc, char **argv)
{
    using namespace std::filesystem;
    using namespace nlohmann;
    using namespace webview;
    using namespace httplib;

    // HTTP Server
    bool ret = guiServer.set_mount_point("/", "./www");
    if (ret == false)
    {
        return EXIT_FAILURE;
    }
    guiServerThread = std::thread([&]() {
        guiServer.listen("localhost", 9002);
    });

    // GUI Window
    guiWindow.set_title("MckSampler");
    guiWindow.set_size(1280, 720, WEBVIEW_HINT_NONE);
    guiWindow.navigate("http://localhost:9002");
    guiWindow.bind("SendMessage", SendMessage);
    guiWindow.bind("GetData", GetData);
    //guiWindowThread = std::thread([&]() {
    //});

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
            m_config = j.get<SP::Config>();
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
    SP::ScanSampleFolder(m_samplePath.string(), m_config.samples);
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
        SP::VerifyConfiguration(m_config);
    }

    SP::VerifyConfiguration(m_config);

    if ((client = jack_client_open("MckSampler", JackNullOption, NULL)) == 0)
    {
        fprintf(stderr, "JACK server not running?\n");
        return EXIT_FAILURE;
    }

    jack_set_process_callback(client, AudioProcess, 0);

    midi_in = jack_port_register(client, "midi_in", JACK_DEFAULT_MIDI_TYPE, JackPortIsInput, 0);
    audio_out_l = jack_port_register(client, "audio_out_l", JACK_DEFAULT_AUDIO_TYPE, JackPortIsOutput, 0);
    audio_out_r = jack_port_register(client, "audio_out_r", JACK_DEFAULT_AUDIO_TYPE, JackPortIsOutput, 0);

    bufferSize = jack_get_buffer_size(client);
    sampleRate = jack_get_sample_rate(client);

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

    if (jack_activate(client))
    {
        fprintf(stderr, "Unable to activate JACK client\n");
        return EXIT_FAILURE;
    }

    // Connect inputs and outputs
    if (m_config.reconnect)
    {
        if (MCK::JACK::SetConnections(client, midi_in, m_config.midiConnections, true) == false)
        {
            printf("Failed to connect port %s\n", jack_port_name(midi_in));
        }
        if (MCK::JACK::SetConnections(client, audio_out_l, m_config.audioLeftConnections, false) == false)
        {
            printf("Failed to connect port %s\n", jack_port_name(audio_out_l));
        }
        if (MCK::JACK::SetConnections(client, audio_out_r, m_config.audioRightConnections, false) == false)
        {
            printf("Failed to connect port %s\n", jack_port_name(audio_out_r));
        }
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

    //sleep(-1);

    guiWindow.run();

    CloseApplication();

    return EXIT_SUCCESS;
}
