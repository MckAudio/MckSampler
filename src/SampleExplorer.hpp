#pragma once
#include "Types.hpp"
#include <vector>
#include <string>
#include <atomic>
#include <mutex>
#include <condition_variable>

namespace mck
{
    class SampleExplorer
    {
        struct PlayState {
            bool active;
            bool stop;
            unsigned idx;
            unsigned len;
            PlayState() : active(false), stop(false), idx(0), len(0) {}
        };

    public:
        SampleExplorer();
        ~SampleExplorer();
        bool Init(unsigned bufferSize, unsigned sampleRate, std::string samplePath);
        
        void RefreshSamples(std::vector<SamplePack> &packs);
        SampleInfo LoadSample(unsigned packIdx, unsigned sampleIdx);
        SampleInfo PlaySample(unsigned packIdx, unsigned sampleIdx);
        void StopSample();
        void ProcessAudio(float *outLeft, float *outRight, unsigned nframes);

    private:
        bool m_isInitialized;
        unsigned m_bufferSize;
        unsigned m_sampleRate;
        std::string m_samplePath;
        std::vector<SamplePack> m_packs;
        std::vector<std::string> m_packPaths;
        SampleInfo m_curInfo;
        float *m_curSampleBuffer;
        std::vector<std::vector<float>> m_waveBuffer;
        PlayState m_state;

        std::atomic<bool> m_isProcessing;
        std::condition_variable m_processCond;
    };
};