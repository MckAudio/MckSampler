#include "Processing.hpp"
#include "helper/JackHelper.hpp"

// System
#include <cstdio>

// Audio
#include <jack/jack.h>
#include <jack/midiport.h>
#include <sndfile.h>
#include <samplerate.h>

static int JackProcess(jack_nframes_t nframes, void *arg)
{
    auto proc = (mck::Processing *)arg;
    proc->ProcessAudioMidi(nframes);
}

mck::Processing::Processing()
    : m_isInitialized(false),
      m_config(),
      m_client(nullptr),
      m_midiIn(nullptr),
      m_midiOut(nullptr),
      m_audioOutL(nullptr),
      m_audioOutR(nullptr),
      m_bufferSize(0),
      m_sampleRate(0)
{
}

mck::Processing::~Processing()
{
    if (m_isInitialized) {
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

    int err = jack_set_process_callback(m_client, JackProcess, 0);

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

    if ((m_client = jack_client_open("MckSampler", JackNullOption, NULL)) == 0)
    {
        std::fprintf(stderr, "JACK server not running?\n");
        return false;
    }

    // Activate Client
    err = jack_activate(m_client);
    if (err)
    {
        std::fprintf(stderr, "Unable to activate JACK client, error code %d\n", err);
        return false;
    }
    // Connect inputs and outputs
    if (m_config.reconnect)
    {
        if (mck::SetConnections(m_client, m_midiIn, m_config.midiInConnections, true) == false)
        {
            std::printf("Failed to connect port %s\n", jack_port_name(m_midiIn));
        }
        if (mck::SetConnections(m_client, m_midiOut, m_config.midiOutConnections, true) == false)
        {
            std::printf("Failed to connect port %s\n", jack_port_name(m_midiOut));
        }
        if (mck::SetConnections(m_client, m_audioOutL, m_config.audioLeftConnections, false) == false)
        {
            std::printf("Failed to connect port %s\n", jack_port_name(m_audioOutL));
        }
        if (mck::SetConnections(m_client, m_audioOutR, m_config.audioRightConnections, false) == false)
        {
            std::printf("Failed to connect port %s\n", jack_port_name(m_audioOutR));
        }
    }


    // Initialized Transport
    if (m_transport.Init(m_sampleRate, m_bufferSize, m_config.tempo) == false) {
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
        if (m_config.reconnect)
        {
            mck::GetConnections(m_client, m_midiIn, m_config.midiInConnections);
            mck::GetConnections(m_client, m_midiOut, m_config.midiOutConnections);
            mck::GetConnections(m_client, m_audioOutL, m_config.audioLeftConnections);
            mck::GetConnections(m_client, m_audioOutR, m_config.audioRightConnections);
        }
        jack_client_close(m_client);
    }

    m_isInitialized = false;
}

int mck::Processing::ProcessAudioMidi(jack_nframes_t nframes)
{
    if (m_isInitialized == false)
    {
        return 0;
    }

    m_transport.Process(m_midiOut, nframes, m_transportState);

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

        if (sysMsg == false && chan == m_config.midiChan)
        {
            /*
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
            {*/
                if ((midiEvent.buffer[0] & 0xf0) == 0x90)
                {
                    for (unsigned j = 0; j < m_config.numPads; j++)
                    {
                        if ((midiEvent.buffer[1] & 0x7f) == m_config.pads[j].tone && m_config.pads[j].available)
                        {
                            m_voices[voiceIdx].playSample = true;
                            m_voices[voiceIdx].startIdx = midiEvent.time;
                            m_voices[voiceIdx].bufferIdx = 0;
                            m_voices[voiceIdx].gain = ((float)(midiEvent.buffer[2] & 0x7f) / 127.0f) * m_config.pads[j].gainLin;
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
    //}

    m_triggerActive = true;
    while (m_trigger.size() > 0)
    {
        unsigned idx = m_trigger[0].first;
        double strength = m_trigger[0].second;
        m_trigger.pop_front();

        if (idx < m_config.numPads)
        {
            if (m_config.pads[idx].available)
            {
                m_voices[voiceIdx].playSample = true;
                m_voices[voiceIdx].startIdx = 0;
                m_voices[voiceIdx].bufferIdx = 0;
                m_voices[voiceIdx].gain = m_config.pads[idx].gainLin * strength;
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

        MCK::AudioSample *s = &m_samples[v.sampleIdx];
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
    while (true)
    {
        if (m_done.load())
        {
            return;
        }

        TransportState ts;
        m_transport.GetRTData(ts);

        //m_gui.SendMessage("transport", "realtime", ts);

        std::this_thread::sleep_for(std::chrono::milliseconds(30));
    }
}