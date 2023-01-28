#pragma once
#include "Types.hpp"
#include <vector>
#include <string>
#include <atomic>
#include <mutex>
#include <condition_variable>
#include <MckHelper/WaveHelper.hpp>
#include "Config.hpp"

namespace mck
{
    class GuiWindow;

    class SampleExplorer
    {
        struct PlayState
        {
            bool active;
            bool stop;
            unsigned idx;
            unsigned pad;
            unsigned len;
            double gainL;
            double gainR;
            PlayState() : active(false), stop(false), idx(0), pad(0), len(0), gainL(0.0f), gainR(0.0f) {}
        };

    public:
        SampleExplorer();
        ~SampleExplorer();
        bool Init(unsigned bufferSize, unsigned sampleRate, std::string samplePath);

        void RefreshSamples(std::vector<SamplePack> &packs);
        WaveInfoDetail LoadSample(unsigned packIdx, unsigned sampleIdx);
        WaveInfoDetail PlaySample(unsigned packIdx, unsigned sampleIdx, unsigned padIdx, sampler::Pad &padData);
        WaveInfoDetail GetSample(unsigned packIdx, unsigned sampleIdx, std::vector<std::vector<float>> &buffer);
        std::string GetSamplePath(unsigned packIdx, unsigned sampleIdx, bool relativePath = true);
        std::string GetSampleName(unsigned packIdx, unsigned sampleIdx);
        std::string GetSampleId(unsigned packIdx, unsigned sampleIdx);
        std::string GetSampleType(unsigned packIdx, unsigned sampleIdx);
        mck::SamplePackSample GetSampleMeta(unsigned packIdx, unsigned sampleIdx);
        SamplePackSample GetSampleMeta(const std::string &id);

        int GetActivePad() {
            return m_state.stop ? -1 : m_state.pad;
        }

        bool ApplyEditCommand(SampleEdit &cmd, GuiWindow *gui);
        void StopSample();
        void ProcessAudio(double *outLeft, double *outRight, unsigned nframes);

    private:
        bool WritePack(std::string path, SamplePack &pack);
        bool UpdatePack(unsigned packIdx);
        bool CreatePack(std::string name);
        bool CreateCategory(std::string name, unsigned packIdx);
        bool ImportSample(std::string path, unsigned packIdx, unsigned categoryIdx, GuiWindow *gui);

        bool m_isInitialized;
        unsigned m_bufferSize;
        unsigned m_sampleRate;
        std::string m_samplePath;
        std::vector<SamplePack> m_packs;
        std::vector<std::string> m_packPaths;
        WaveInfoDetail m_waveInfo[2];
        std::vector<std::vector<float>> m_waveBuffer[2];
        size_t m_curWave;
        PlayState m_state;

        std::atomic<bool> m_isProcessing;
        std::condition_variable m_processCond;
    };
}
