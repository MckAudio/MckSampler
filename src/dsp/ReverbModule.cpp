#include "ReverbModule.hpp"
#include <algorithm>

namespace mck::dsp
{
    ReverbModule::ReverbModule()
    {
    }
    ReverbModule::~ReverbModule()
    {
    }

    void ReverbModule::init(double sampleRate, size_t bufferSize, size_t channelCount)
    {
        m_reverb.Init(sampleRate);
        //resizeBuffer(bufferSize, channelCount);
        m_tmp.resize(channelCount, 0.0f);
        m_bufferSize = bufferSize;
        m_channelCount = channelCount;
        m_sampleRate = sampleRate;
        m_isInitialized = true;
    }

    void ReverbModule::processBlock(const double **readPtr, double **writePtr)
    {
        double invMix = 1.0 - m_mix;
        if (m_channelCount >= 2)
        {
            for (size_t i = 0; i < m_bufferSize; i++)
            {
                m_reverb.Process(readPtr[0][i], readPtr[1][i], &(m_tmp[0]), &(m_tmp[1]));
                writePtr[0][i] = m_mix * m_tmp[0] + invMix * readPtr[0][i];
                writePtr[1][i] = m_mix * m_tmp[1] + invMix * readPtr[1][i];
            }
        }
    }
    void ReverbModule::setMix(double mix)
    {
        m_mix = std::clamp(mix, 0.0, 1.0);
    }
    void ReverbModule::setFeedback(double feedback)
    {
        m_feedback = std::clamp(feedback, 0.0, 1.0);
        m_reverb.SetFeedback(m_feedback);
    }
    void ReverbModule::setCutoff(double cutoff)
    {
        if (m_isInitialized)
        {
            m_cutoff = std::clamp(cutoff, 0.0, m_sampleRate / 2.0);
            m_reverb.SetLpFreq(m_cutoff);
        }
    }
    void ReverbModule::resizeBuffer(size_t bufferSize, size_t channelCount)
    {
        if (channelCount != m_channelCount)
        {
            m_buffer.resize(channelCount);
            for (auto &buf : m_buffer)
            {
                buf.resize(m_bufferSize, 0.0);
            }
        }
        else if (bufferSize != m_bufferSize)
        {
            for (auto &buf : m_buffer)
            {
                buf.resize(m_bufferSize, 0.0);
            }
        }

        m_bufferSize = bufferSize;
        m_channelCount = channelCount;
    }

}