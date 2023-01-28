#pragma once

#include <JuceHeader.h>

#include <vector>
#include <deque>
#include <string>
#include <thread>
#include <atomic>
#include <mutex>
#include <condition_variable>

#include <jack/types.h>
#include <concurrentqueue.h>

#include <MckHelper/Transport.hpp>
#include <MckHelper/DspHelper.hpp>

#include "Types.hpp"
#include "Config.hpp"
#include "ConfigFile.hpp"

namespace mck
{
    struct Meter {
        double left{0.0};
        double right{0.0};
    };

    const unsigned SAMPLER_NUM_PADS = 8;
    const unsigned SAMPLER_VOICES_PER_PAD = 4;

    class SampleExplorer;

    class Processing
    {
    protected:
        Processing();

    public:
        Processing(Processing &) = delete;

        ~Processing();

        void operator=(const Processing &) = delete;

        static Processing *GetInstance();

        int ProcessAudioMidi(jack_nframes_t nframes);
        void Process(float *outL, float *outR, unsigned nSamples);

        bool Init(unsigned sampleRate, unsigned blockSize);
        void Close();

        void Trigger(size_t idx, double strength);

        void SetLevel(size_t idx, double level);

        void SetPan(size_t idx, double pan);

        void SetCompression(size_t idx, bool active, double threshold, double ratio);
        
        void SetDelay(size_t idx, bool active, double timeMs, double mix, double feedback);

        void SetActivePad(size_t idx);

        void SetSample(SampleCommand cmd);

        Meter GetLevelLin() {
            return {m_levelLeft.load(), m_levelRight.load()};
        }

        Meter GetLevelDb() {
            return {
                LinToDb(m_levelLeft.load()),
                LinToDb(m_levelRight.load())
            };
        }

        sampler::Config GetCurrentConfig()
        {
            return m_config[m_curConfig];
        }

        // void ReceiveMessage(mck::Message &msg);

        class JUCE_API Listener
        {
        public:
            virtual ~Listener() = default;

            virtual void configChanged(const sampler::Config &config) = 0;

            virtual void samplesChanged(const std::vector<SamplePack> &samples) {}
        };

        void addListener(Listener *newListener)
        {
            configListeners.add(newListener);
            newListener->configChanged(m_config[m_curConfig]);
            newListener->samplesChanged(m_samplePacks);
        };
        void removeListener(Listener *listener) { configListeners.remove(listener); }

    private:
        void TransportThread();
        bool PrepareSamples();
        bool AssignSample(SampleCommand cmd);
        void SetConfiguration(sampler::Config &config, bool connect = false);
        void CalcLevels(jack_default_audio_sample_t *inL, jack_default_audio_sample_t *inR, unsigned nframes);

        ListenerList<Listener> configListeners;

        // INIT Members
        bool m_isInitialized;
        std::atomic<bool> m_done;
        std::atomic<bool> m_isProcessing;

        // DATA Members
        sampler::Config m_config[2];
        size_t m_curConfig;
        size_t m_newConfig;
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
        // std::string m_samplePath;
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

        std::atomic<double> m_levelLeft{0.0};
        std::atomic<double> m_levelRight{0.0};

        double m_levelCoeff{1.0/24000.0};
    };
} // namespace mck
