#pragma once
#include <vector>
#include <Effects/reverbsc.h>

namespace mck
{
    namespace dsp
    {
        class ReverbModule
        {
            public:
            ReverbModule();
            ~ReverbModule();

            void init(double sampleRate, size_t bufferSize, size_t channelCount);

            void processBlock(const double **readPtr, double **writePtr);

            void setMix(double mix);
            void setFeedback(double feedback);
            void setCutoff(double cutoff);

            private:
            void resizeBuffer(size_t bufferSize, size_t channelCount);

            double **m_buffer;
            float *m_tmp;

            bool m_isInitialized { false };
            double m_sampleRate { 0 };
            size_t m_bufferSize { 0 };
            size_t m_channelCount { 0 };

            double m_mix { 0.0 };
            double m_feedback { 0.5 };
            double m_cutoff { 10000.0 };

            daisysp::ReverbSc m_reverb;
        };
    }
}