#include "ReverbModule.hpp"
#include <algorithm>
#include <cstring>

namespace mck::dsp
{
    ReverbModule::ReverbModule()
    {
    }
    ReverbModule::~ReverbModule()
    {
        if (m_isInitialized) {
            m_isInitialized = false;
            for (size_t i = 0; i < m_channelCount; i++)
            {
                delete[] m_buffer[i];
                m_buffer[i] = nullptr;
            }
            delete[] m_buffer;
            m_buffer = nullptr;

            delete[] m_tmp;
            m_tmp = nullptr;
        }
    }

    void ReverbModule::init(double sampleRate, size_t bufferSize, size_t channelCount)
    {
        m_reverb.Init(sampleRate);
        //resizeBuffer(bufferSize, channelCount);
        m_tmp = new float[channelCount];
        memset(m_tmp, 0, 2 * sizeof(float));
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
            for (size_t i = 0; i < m_channelCount; i++)
            {
                memcpy(writePtr[i], readPtr[i], m_bufferSize * sizeof(double));
            }
            /*
            for (size_t i = 0; i < m_bufferSize; i++)
            {
                m_reverb.Process(readPtr[0][i], readPtr[1][i], &(m_tmp[0]), &(m_tmp[1]));
                writePtr[0][i] = m_mix * m_tmp[0] + invMix * readPtr[0][i];
                writePtr[1][i] = m_mix * m_tmp[1] + invMix * readPtr[1][i];
            }*/
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
            m_buffer = new double*[channelCount];
            for (size_t i = 0; i < channelCount; i++)
            {
                m_buffer[i] = new double[m_bufferSize]; 
                memset(m_buffer + i, 0, bufferSize * sizeof(double));
            }
        }
        else if (bufferSize != m_bufferSize)
        {
            for (size_t i = 0; i < channelCount; i++)
            {
                m_buffer[i] = new double[m_bufferSize];
                memset(m_buffer + i, 0, bufferSize * sizeof(double));
            }
        }

        m_bufferSize = bufferSize;
        m_channelCount = channelCount;
    }

}