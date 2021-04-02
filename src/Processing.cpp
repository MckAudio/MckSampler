#include "Processing.hpp"
#include "GuiWindow.hpp"
#include "helper/JackHelper.hpp"
#include "SampleExplorer.hpp"

// System
#include <cstdio>
#include <filesystem>
#include <nlohmann/json.hpp>

// Audio
#include <jack/jack.h>
#include <jack/midiport.h>
#include <sndfile.h>
#include <samplerate.h>

static int JackProcess(jack_nframes_t nframes, void *arg)
{
    auto proc = (mck::Processing *)arg;
    return proc->ProcessAudioMidi(nframes);
}

mck::Processing::Processing()
    : m_gui(nullptr),
      m_isInitialized(false),
      m_config(),
      m_curConfig(0),
      m_newConfig(1),
      m_updateConfig(false),
      m_configFile(),
      m_configPath(""),
      m_client(nullptr),
      m_midiIn(nullptr),
      m_midiOut(nullptr),
      m_audioOutL(nullptr),
      m_audioOutR(nullptr),
      m_bufferSize(0),
      m_transportStep(-1),
      m_transportRate(0),
      m_samplePath(""),
      m_sampleRate(0),
      m_numVoices(0),
      m_voiceIdx(0),
      m_triggerActive(false),
      m_samplePackPath(""),
      m_sampleExplorer(nullptr),
      m_samplePacks()
{
}

mck::Processing::~Processing()
{
    if (m_isInitialized)
    {
        Close();
    }
}

bool mck::Processing::Init()
{
    if (m_isInitialized)
    {
        std::fprintf(stderr, "Processing is already initialized\n");
        return false;
    }

    // 1A - Load Configuration
    std::string homeDir = ConfigFile::GetHomeDir();
    std::filesystem::path configPath(homeDir);
    configPath.append(".mck").append("sampler").append("config.json");
    m_configPath = configPath.string();
    if (m_configFile.ReadFile(m_configPath))
    {
        m_configFile.GetConfig(m_config[m_curConfig]);
    }
    if (m_config[m_curConfig].numPads == 0)
    {
        m_config[m_curConfig].pads.resize(16);
    }

    // 1B - Scan Samples
    std::filesystem::path samplePath(homeDir);
    samplePath.append(".mck").append("sampler").append("audio");
    if (std::filesystem::exists(samplePath) == false)
    {
        std::filesystem::create_directories(samplePath);
    }
    mck::sampler::ScanSampleFolder(samplePath.string(), m_config[m_curConfig].samples);
    m_config[m_curConfig].numSamples = m_config[m_curConfig].samples.size();
    m_samplePath = samplePath.string();

    // 1C - Update File
    mck::sampler::VerifyConfiguration(m_config[m_curConfig]);
    m_configFile.SetConfig(m_config[m_curConfig]);
    m_configFile.WriteFile(m_configPath);

    // 2 - Init JACK
    if ((m_client = jack_client_open("MckSampler", JackNullOption, NULL)) == 0)
    {
        std::fprintf(stderr, "JACK server not running?\n");
        return false;
    }
    int err = jack_set_process_callback(m_client, JackProcess, this);

    if (err)
    {
        std::fprintf(stderr, "Failed to set JACK callback, error code %d\n", err);
        return false;
    }

    m_midiIn = jack_port_register(m_client, "midi_in", JACK_DEFAULT_MIDI_TYPE, JackPortIsInput, 0);
    m_midiOut = jack_port_register(m_client, "midi_out", JACK_DEFAULT_MIDI_TYPE, JackPortIsOutput, 0);
    m_audioOutL = jack_port_register(m_client, "audio_out_l", JACK_DEFAULT_AUDIO_TYPE, JackPortIsOutput, 0);
    m_audioOutR = jack_port_register(m_client, "audio_out_r", JACK_DEFAULT_AUDIO_TYPE, JackPortIsOutput, 0);

    m_bufferSize = jack_get_buffer_size(m_client);
    m_sampleRate = jack_get_sample_rate(m_client);
    m_transportRate = m_sampleRate;

    // 3A - Prepare Samples & Voices

    if (PrepareSamples() == false)
    {
        return false;
    }


    // 3B - Scan Sample Packs
    std::filesystem::path samplePackPath(homeDir);
    samplePackPath.append(".local").append("share").append("mck").append("sampler");
    m_samplePackPath = samplePackPath.string();
    m_sampleExplorer = new SampleExplorer();
    if (m_sampleExplorer->Init(m_bufferSize, m_sampleRate, m_samplePackPath) == false) {
        std::fprintf(stderr, "Failed to init SampleExplorer!\n");
        return false;
    }
    m_sampleExplorer->RefreshSamples(m_samplePacks);

    // 4 - Start JACK Processing
    err = jack_activate(m_client);
    if (err)
    {
        std::fprintf(stderr, "Unable to activate JACK client, error code %d\n", err);
        return false;
    }
    // Connect inputs and outputs
    if (m_config[m_curConfig].reconnect)
    {
        if (jack::SetConnections(m_client, m_midiIn, m_config[m_curConfig].midiInConnections, true) == false)
        {
            std::printf("Failed to connect port %s\n", jack_port_name(m_midiIn));
        }
        if (jack::SetConnections(m_client, m_midiOut, m_config[m_curConfig].midiOutConnections, true) == false)
        {
            std::printf("Failed to connect port %s\n", jack_port_name(m_midiOut));
        }
        if (jack::SetConnections(m_client, m_audioOutL, m_config[m_curConfig].audioLeftConnections, false) == false)
        {
            std::printf("Failed to connect port %s\n", jack_port_name(m_audioOutL));
        }
        if (jack::SetConnections(m_client, m_audioOutR, m_config[m_curConfig].audioRightConnections, false) == false)
        {
            std::printf("Failed to connect port %s\n", jack_port_name(m_audioOutR));
        }
    }

    // 5 - Initialized Transport

    if (m_transport.Init(m_client, m_config[m_curConfig].tempo) == false)
    {
        return false;
    }
    m_transportThread = std::thread(&mck::Processing::TransportThread, this);

    m_isInitialized = true;
    return true;
}

void mck::Processing::Close()
{
    m_done = true;
    if (m_client != nullptr)
    {
        // Save Connections
        if (m_config[m_curConfig].reconnect)
        {
            jack::GetConnections(m_client, m_midiIn, m_config[m_curConfig].midiInConnections);
            jack::GetConnections(m_client, m_midiOut, m_config[m_curConfig].midiOutConnections);
            jack::GetConnections(m_client, m_audioOutL, m_config[m_curConfig].audioLeftConnections);
            jack::GetConnections(m_client, m_audioOutR, m_config[m_curConfig].audioRightConnections);
        }
        jack_client_close(m_client);
    }

    // Save File
    m_configFile.SetConfig(m_config[m_curConfig]);
    m_configFile.WriteFile(m_configPath);

    for (auto &s : m_samples)
    {
        if (s.buffer != nullptr)
        {
            delete s.buffer;
            s.buffer = nullptr;
        }
    }

    m_transportCond.notify_all();
    if (m_transportThread.joinable())
    {
        m_transportThread.join();
    }

    m_isInitialized = false;
}

void mck::Processing::ReceiveMessage(mck::Message &msg)
{
    if (msg.section == "pads")
    {
        if (msg.msgType == "trigger")
        {
            try
            {
                mck::TriggerData data = nlohmann::json::parse(msg.data);
                std::printf("Triggering PAD #%d\n", data.index + 1);
                m_triggerQueue.try_enqueue(std::pair<unsigned, double>(data.index, data.strength));
            }
            catch (std::exception &e)
            {
                return;
            }
        }
    }
    else if (msg.section == "transport")
    {
        if (msg.msgType == "command")
        {
            try
            {
                mck::TransportCommand cmd = nlohmann::json::parse(msg.data);
                m_transport.ApplyCommand(cmd);
            }
            catch (std::exception &e)
            {
                return;
            }
        }
    }
    else if (msg.section == "data")
    {
        if (msg.msgType == "get")
        {
            m_gui->SendMessage("data", "full", m_config[m_curConfig]);
        }
        else if (msg.msgType == "patch")
        {
            char newConfig = 1 - m_curConfig;
            auto config = m_config[m_curConfig];
            try
            {
                nlohmann::json j = config;
                nlohmann::json jPatch = nlohmann::json::parse(msg.data);
                config = j.patch(jPatch);
            }
            catch (std::exception &e)
            {
                std::fprintf(stderr, "Failed to apply data patch: %s", e.what());
                m_gui->SendMessage("data", "full", m_config[m_curConfig]);
                return;
            }
            mck::sampler::VerifyConfiguration(config);
            m_config[1 - m_curConfig] = config;
            m_newConfig = 1 - m_curConfig;
            m_updateConfig = true;
            m_gui->SendMessage("data", "full", config);
        }
    }
    else if (msg.section == "samples")
    {
        if (msg.msgType == "get")
        {
            m_sampleExplorer->RefreshSamples(m_samplePacks);
            m_gui->SendMessage("samples", "packs", m_samplePacks);
        }
    }
}

void mck::Processing::SetGuiPtr(GuiWindow *gui)
{
    m_gui = gui;
}

int mck::Processing::ProcessAudioMidi(jack_nframes_t nframes)
{
    if (m_isInitialized == false)
    {
        return 0;
    }

    if (m_updateConfig.load())
    {
        m_curConfig = m_newConfig;
        m_updateConfig = false;
    }

    TransportState ts;
    m_transport.Process(m_midiOut, nframes, ts);

    int stepIdx = -1;
    if (ts.state == TS_RUNNING)
    {
        stepIdx = ts.beat * 4;
        stepIdx += (int)std::floor((double)ts.pulse / (double)ts.nPulses * 4.0);
        stepIdx %= 16;
    }

    void *midi_buf = jack_port_get_buffer(m_midiIn, nframes);

    jack_nframes_t midiEventCount = jack_midi_get_event_count(midi_buf);
    jack_midi_event_t midiEvent;

    bool sysMsg = false;
    unsigned char chan = 0;

    for (unsigned i = 0; i < midiEventCount; i++)
    {
        jack_midi_event_get(&midiEvent, midi_buf, i);
        sysMsg = (midiEvent.buffer[0] & 0xf0) == 0xf0;
        chan = (midiEvent.buffer[0] & 0x0f);

        if (sysMsg == false && chan == m_config[m_curConfig].midiChan)
        {
            /*
            if (configMode == CONFIG_PADS)
            {
                if ((midiEvent.buffer[0] & 0xf0) == 0x90)
                {
                    m_config[m_curConfig].pads[configIdx].tone = (midiEvent.buffer[1] & 0x7f);
                    printf("Saved note %X for pad #%d.\n\n", m_config[m_curConfig].pads[configIdx].tone, configIdx + 1);
                    configIdx += 1;
                    if (configIdx >= m_config[m_curConfig].numPads)
                    {
                        printf("Finished configuration, entering play mode...\n");
                        configMode = false;
                        break;
                    }
                    printf("Please play note for pad #%d on MIDI channel %d:\n", configIdx + 1, m_config[m_curConfig].midiChan + 1);
                    break;
                }
            }
            else if (configMode == CONFIG_CTRLS)
            {
                if ((midiEvent.buffer[0] & 0xf0) == 0xb0)
                {

                    if (configIdx > 0 && (midiEvent.buffer[1] & 0x7f) == m_config[m_curConfig].pads[configIdx - 1].ctrl)
                    {
                        continue;
                    }
                    m_config[m_curConfig].pads[configIdx].ctrl = (midiEvent.buffer[1] & 0x7f);
                    printf("Saved control %X for pad #%d.\n\n", m_config[m_curConfig].pads[configIdx].ctrl, configIdx + 1);
                    configIdx += 1;
                    if (configIdx >= m_config[m_curConfig].numPads)
                    {
                        printf("Finished configuration, entering play mode...\n");
                        configMode = CONFIG_NONE;
                        break;
                    }
                    printf("Please turn the controller for pad #%d on MIDI channel %d:\n", configIdx + 1, m_config[m_curConfig].midiChan + 1);
                    break;
                }
            }
            else
            {*/
            if ((midiEvent.buffer[0] & 0xf0) == 0x90)
            {
                for (unsigned j = 0; j < m_config[m_curConfig].numPads; j++)
                {
                    if ((midiEvent.buffer[1] & 0x7f) == m_config[m_curConfig].pads[j].tone && m_config[m_curConfig].pads[j].available)
                    {
                        m_voices[m_voiceIdx].playSample = true;
                        m_voices[m_voiceIdx].startIdx = midiEvent.time;
                        m_voices[m_voiceIdx].bufferIdx = 0;
                        m_voices[m_voiceIdx].gain = ((float)(midiEvent.buffer[2] & 0x7f) / 127.0f) * m_config[m_curConfig].pads[j].gainLin;
                        m_voices[m_voiceIdx].sampleIdx = j;
                        m_voices[m_voiceIdx].pitch = m_config[m_curConfig].pads[j].pitch;

                        m_voiceIdx = (m_voiceIdx + 1) % m_numVoices;
                    }
                }
            }
            else if ((midiEvent.buffer[0] & 0xf0) == 0xb0)
            {
                for (unsigned j = 0; j < m_config[m_curConfig].numPads; j++)
                {
                    if ((midiEvent.buffer[1] & 0x7f) == m_config[m_curConfig].pads[j].ctrl)
                    {
                        m_config[m_curConfig].pads[j].gain = (float)(midiEvent.buffer[2] & 0x7f) / 127.0f;
                        //m_config[m_curConfig].pads[j].pitch = ((float)(midiEvent.buffer[2] & 0x7f) / 127.0f) * 1.5f + 0.5;
                    }
                }
            }
        }
    }
    //}

    // GUI DRUM TRIGGER
    std::pair<unsigned, double> trigger;
    while (m_triggerQueue.try_dequeue(trigger))
    {
        unsigned idx = trigger.first;
        double strength = trigger.second;

        if (idx < m_config[m_curConfig].numPads)
        {
            if (m_config[m_curConfig].pads[idx].available)
            {
                m_voices[m_voiceIdx].playSample = true;
                m_voices[m_voiceIdx].startIdx = 0;
                m_voices[m_voiceIdx].bufferIdx = 0;
                m_voices[m_voiceIdx].gain = m_config[m_curConfig].pads[idx].gainLin * strength;
                m_voices[m_voiceIdx].sampleIdx = idx;
                m_voices[m_voiceIdx].pitch = m_config[m_curConfig].pads[idx].pitch;

                m_voiceIdx = (m_voiceIdx + 1) % m_numVoices;
            }
        }
    }

    // Transport TRIGGER
    if (stepIdx >= 0 && stepIdx != m_transportStep)
    {
        unsigned padIdx = 0;
        unsigned patternIdx = (unsigned)std::floor((double)stepIdx / 16.0);
        for (auto &pad : m_config[m_curConfig].pads)
        {
            if (pad.available == false)
            {
                padIdx += 1;
                continue;
            }
            if (pad.nPatterns == 0)
            {
                padIdx += 1;
                continue;
            }

            unsigned curPatIdx = patternIdx % pad.nPatterns;
            unsigned curStepIdx = stepIdx % pad.patterns[curPatIdx].nSteps;

            if (pad.patterns[curPatIdx].steps[curStepIdx].active)
            {
                double strength = (double)pad.patterns[curPatIdx].steps[curStepIdx].velocity / 127.0;
                m_voices[m_voiceIdx].playSample = true;
                m_voices[m_voiceIdx].startIdx = ts.pulseIdx % m_bufferSize;
                m_voices[m_voiceIdx].bufferIdx = 0;
                m_voices[m_voiceIdx].gain = pad.gainLin * strength;
                m_voices[m_voiceIdx].sampleIdx = padIdx;
                m_voices[m_voiceIdx].pitch = pad.pitch;

                m_voiceIdx = (m_voiceIdx + 1) % m_numVoices;
            }
            padIdx += 1;
        }
        m_transportStep = stepIdx;
        m_transportState = ts;
        m_transportCond.notify_one();
        m_transportRate += m_bufferSize;
    } else if (m_transportRate >= m_sampleRate || ts.state != m_transportState.state) {
        m_transportState = ts;
        m_transportCond.notify_one();
        m_transportRate = 0;
    } else {
        m_transportRate += m_bufferSize;
    }

    /*
    if (resetSample)
    {
        sf_seek(sndFile, 0, SEEK_SET);
        resetSample = false;
    }*/

    jack_default_audio_sample_t *out_l = (jack_default_audio_sample_t *)jack_port_get_buffer(m_audioOutL, nframes);
    jack_default_audio_sample_t *out_r = (jack_default_audio_sample_t *)jack_port_get_buffer(m_audioOutR, nframes);

    memset(out_l, 0, nframes * sizeof(jack_default_audio_sample_t));
    memset(out_r, 0, nframes * sizeof(jack_default_audio_sample_t));

    unsigned len = 0;
    for (auto &v : m_voices)
    {
        if (v.playSample == false)
        {
            continue;
        }

        mck::AudioSample *s = &m_samples[v.sampleIdx];
        /*
        for (unsigned i = 0; i < s->numChans; i++)
        {
            memset(s->pitchBuffer[i], 0, bufferSize * sizeof(float));
        }*/
        len = std::min(m_bufferSize, s->numFrames - v.bufferIdx) - v.startIdx;

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

void mck::Processing::TransportThread()
{
    std::unique_lock lock(m_transportMutex);
    while (true)
    {
        m_transportCond.wait(lock);

        if (m_done.load())
        {
            return;
        }

        if (m_gui != nullptr)
        {
            m_gui->SendMessage("transport", "realtime", m_transportState);
        }
    }
}

bool mck::Processing::PrepareSamples()
{
    if (m_isInitialized)
    {
        return false;
    }

    // Prepare Sound Files and Voices
    m_numVoices = 4 * m_config[m_curConfig].numPads;
    m_voiceIdx = 0;

    m_samples.resize(m_config[m_curConfig].numPads);
    m_voices.resize(m_numVoices);

    SNDFILE *tmpSnd;
    SF_INFO tmpInfo;
    SRC_DATA tmpSrc;
    float *tmpBuf;
    float *tmpSrcBuf;
    unsigned numSrcFrames;

    for (unsigned i = 0; i < m_config[m_curConfig].numPads; i++)
    {
        if (m_config[m_curConfig].pads[i].sampleIdx >= m_config[m_curConfig].numSamples)
        {
            m_config[m_curConfig].pads[i].available = false;
            continue;
        }
        m_config[m_curConfig].pads[i].available = true;
        tmpInfo.format = 0;
        tmpSnd = sf_open(m_config[m_curConfig].samples[m_config[m_curConfig].pads[i].sampleIdx].fullPath.c_str(), SFM_READ, &tmpInfo);
        m_samples[i].numChans = tmpInfo.channels;
        m_samples[i].numFrames = tmpInfo.frames;
        tmpBuf = new float[tmpInfo.channels * tmpInfo.frames];
        numSrcFrames = sf_readf_float(tmpSnd, tmpBuf, tmpInfo.frames);
        m_samples[i].numFrames = numSrcFrames;
        if (tmpInfo.samplerate != m_sampleRate)
        {
            // Sample Rate Conversion
            double convCoeff = (double)m_sampleRate / (double)tmpInfo.samplerate;
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
                std::fprintf(stderr, "Failed to apply samplerate conversion to sample %s:\n%s\nExiting...", m_config[m_curConfig].pads[i].sample.c_str(), src_strerror(err));
                delete tmpBuf;
                delete tmpSrcBuf;
                return false;
            }
            m_samples[i].numFrames = tmpSrc.output_frames_gen;
            /*
            SNDFILE *outSnd;
            SF_INFO outInf;
            outInf.channels = m_samples[i].numChans;
            outInf.format = SF_FORMAT_WAV | SF_FORMAT_FLOAT;
            outInf.frames = m_samples[i].numFrames;
            outInf.samplerate = sampleRate;
            std::filesystem::path outPath(m_config[m_curConfig].samples[m_config[m_curConfig].pads[i].sampleIdx].fullPath);
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

    return true;
}