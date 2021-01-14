#pragma once

#include <vector>
#include <deque>
#include <string>
#include <thread>
#include <atomic>
#include <mutex>
#include <condition_variable>

#include <jack/types.h>

#include "helper/Transport.hpp"
#include "Types.hpp"
#include "Config.hpp"
#include "ConfigFile.hpp"

namespace mck
{
    class Processing
    {
    public:
        Processing();
        ~Processing();

        int ProcessAudioMidi(jack_nframes_t nframes);

        bool Init();
        void Close();

    private:
        void TransportThread();
        bool PrepareSamples();

        // INIT Members
        bool m_isInitialized;
        std::atomic<bool> m_done;

        // DATA Members
        sampler::Config m_config;
        ConfigFile m_configFile;
        std::string m_configPath;

        // JACK Members
        jack_client_t *m_client;
        jack_port_t *m_midiIn;
        jack_port_t *m_midiOut;
        jack_port_t *m_audioOutL;
        jack_port_t *m_audioOutR;
        jack_nframes_t m_bufferSize;
        jack_nframes_t m_sampleRate;

        // Transport Members
        Transport m_transport;
        TransportState m_transportState;
        std::thread m_transportThread;
        std::mutex m_transportMutex;

        // Wav Files
        std::string m_samplePath;
        std::vector<MCK::AudioSample> m_samples;
        std::vector<MCK::AudioVoice> m_voices;
        unsigned m_numVoices;
        unsigned m_voiceIdx;

        // Pad Trigger
        std::deque<std::pair<unsigned, double>> m_trigger;
        std::mutex m_triggerMutex;
        std::atomic<bool> m_triggerActive;
        std::condition_variable m_triggerCond;
    };
} // namespace mck