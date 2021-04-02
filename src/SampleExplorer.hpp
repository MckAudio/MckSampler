#pragma once
#include "Types.hpp"
#include <vector>
#include <string>

namespace mck
{
    class SampleExplorer
    {
    public:
        SampleExplorer();
        ~SampleExplorer();
        bool Init(unsigned bufferSize, unsigned sampleRate, std::string samplePath);
        
        void RefreshSamples(std::vector<SamplePack> &packs);
        bool PlaySample(unsigned packIdx, unsigned sampleIdx);
        void ProcessAudio(float **output, unsigned numChannels);

    private:
        bool m_isInitialized;
        unsigned m_bufferSize;
        unsigned m_sampleRate;
        std::string m_samplePath;
        std::vector<SamplePack> m_packs;
        std::vector<std::string> m_packPaths;
    };
};