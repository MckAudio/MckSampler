#pragma once

#include <vector>
#include <cstddef>
#include "OnePoleFilter.hpp"

namespace MckDsp
{
    class DelayModule
    {
    public:
        DelayModule();
        ~DelayModule();

        void prepareToPlay(double sampleRate, int samplesPerBlock);

        double processSample(double in);

        void processBlock(const double *readPtr, double *writePtr);

        void setMaxDelayInMs(double maxDelayInMs);
        double getMaxDelayInMs() { return m_maxDelayInMs; };

        void setDelayInMs(double delayInMs);

        void setMix(double mix);

        void setFeedback(double fb);

        void setLowPass(bool active, double freq = 20000.0);

        void setHighPass(bool active, double freq = 10.0);

    private:
        void resizeBuffer(double sampleRate, double maxDelayInMs);

        OnePoleFilter m_lpFilter{};
        OnePoleFilter m_hpFilter{};

        size_t m_samplesPerBlock{0};

        double m_sampleRate{0};

        double m_mix{0.0};
        double m_fb{0.0};

        double m_maxDelayInMs{1000.0};
        unsigned m_maxDelayInSamples{0};

        double m_delayInMs{0.0};
        unsigned m_delayInSamples{0};

        unsigned m_len{0};
        unsigned m_idx{0};
        std::vector<double> m_buf{};
    };
}