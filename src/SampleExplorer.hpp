#pragma once
#include "Types.hpp"
#include <vector>
#include <string>
#include <atomic>
#include <mutex>
#include <condition_variable>
#include "helper/WaveHelper.hpp"

namespace mck
{
    class SampleExplorer
    {
        struct PlayState
        {
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
        WaveInfoDetail LoadSample(unsigned packIdx, unsigned sampleIdx);
        WaveInfoDetail PlaySample(unsigned packIdx, unsigned sampleIdx);
        WaveInfoDetail GetSample(unsigned packIdx, unsigned sampleIdx, std::vector<std::vector<float>> &buffer);
        std::string GetSamplePath(unsigned packIdx, unsigned sampleIdx, bool relativePath = true);
        std::string GetSampleName(unsigned packIdx, unsigned sampleIdx);
        void StopSample();
        void ProcessAudio(float *outLeft, float *outRight, unsigned nframes);

    private:
        bool m_isInitialized;
        unsigned m_bufferSize;
        unsigned m_sampleRate;
        std::string m_samplePath;
        std::vector<SamplePack> m_packs;
        std::vector<std::string> m_packPaths;
        WaveInfoDetail m_waveInfo[2];
        std::vector<std::vector<float>> m_waveBuffer[2];
        char m_curWave;
        PlayState m_state;

        std::atomic<bool> m_isProcessing;
        std::condition_variable m_processCond;
    };
};