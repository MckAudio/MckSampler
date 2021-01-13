#pragma once

#include <thread>
#include <atomic>
#include <mutex>
#include <condition_variable>

#include <jack/types.h>

#include "helper/Transport.hpp"
#include "Types.hpp"

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

        // INIT Members
        bool m_isInitialized;
        std::atomic<bool> m_done;

        // DATA Members
        mck::sampler::Config m_config;

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
    };
} // namespace mck