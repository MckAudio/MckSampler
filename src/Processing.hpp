#pragma once

#include <vector>
#include <deque>
#include <string>
#include <thread>
#include <atomic>
#include <mutex>
#include <condition_variable>

#include <jack/types.h>
#include <concurrentqueue.h>

#include <GuiWindow.hpp>
#include "helper/Transport.hpp"
#include "Types.hpp"
#include "Config.hpp"
#include "ConfigFile.hpp"

namespace mck
{

    const unsigned SAMPLER_NUM_PADS = 16;
    const unsigned SAMPLER_VOICES_PER_PAD = 4;

    class SampleExplorer;

    class Processing : GuiBase
    {
    public:
        Processing();
        ~Processing();

        int ProcessAudioMidi(jack_nframes_t nframes);

        bool Init();
        void Close();

        void ReceiveMessage(mck::Message &msg);

        void SetGuiPtr(GuiWindow *gui);

    private:
        void TransportThread();
        bool PrepareSamples();
        bool AssignSample(SampleCommand cmd);
        void SetConfiguration(sampler::Config &config, bool connect = false);

        // GUI Pointer
        GuiWindow *m_gui;

        // INIT Members
        bool m_isInitialized;
        std::atomic<bool> m_done;
        std::atomic<bool> m_isProcessing;

        // DATA Members
        sampler::Config m_config[2];
        char m_curConfig;
        char m_newConfig;
        std::atomic<bool> m_updateConfig;
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
        std::condition_variable m_transportCond;
        int m_transportStep;
        unsigned m_transportRate;

        // Wav Files
        //std::string m_samplePath;
        std::vector<mck::AudioSample> m_samples;
        std::vector<mck::AudioVoice> m_voices;
        unsigned m_numVoices;
        unsigned m_voiceIdx;

        // Pad Trigger
        std::deque<std::pair<unsigned, double>> m_trigger;
        std::mutex m_triggerMutex;
        std::atomic<bool> m_triggerActive;
        std::condition_variable m_triggerCond;

        moodycamel::ConcurrentQueue<std::pair<unsigned, double>> m_triggerQueue;

        // Sample Explorer
        std::string m_samplePackPath;
        SampleExplorer *m_sampleExplorer;
        std::vector<SamplePack> m_samplePacks;

        std::condition_variable m_processCond;
    };
} // namespace mck