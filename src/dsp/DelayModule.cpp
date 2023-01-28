#include "DelayModule.hpp"

#include <cmath>

namespace mck::dsp
{
    DelayModule::DelayModule()
    {
    }

    DelayModule::~DelayModule()
    {
    }

    void DelayModule::prepareToPlay(double sampleRate, int samplesPerBlock, int channelCount)
    {
        m_lpFilter.prepareToPlay(sampleRate, samplesPerBlock);
        m_hpFilter.prepareToPlay(sampleRate, samplesPerBlock);
        resizeBuffer(sampleRate, m_maxDelayInMs);
        m_samplesPerBlock = samplesPerBlock;
    }

    double DelayModule::processSample(double in)
    {
        if (m_len == 0)
        {
            return in;
        }

        unsigned readIdx = (m_idx + m_len - m_delayInSamples) % m_len;

        m_buf[m_idx] = m_lpFilter.processSample(m_hpFilter.processSample(m_fb * m_buf[readIdx] + in));
        m_idx = (m_idx + 1) % m_len;

        return m_mix * m_buf[readIdx] + (1.0 - m_mix) * in;
    }

    void DelayModule::processBlock(const double *readPtr, double *writePtr)
    {
        if (m_len == 0)
        {
            return;
        }
        unsigned readIdx = 0;
        for (size_t s = 0; s < m_samplesPerBlock; s++)
        {
            readIdx = (m_idx + m_len - m_delayInSamples) % m_len;
            m_buf[m_idx] = m_lpFilter.processSample(m_hpFilter.processSample(m_fb * m_buf[readIdx] + readPtr[s]));
            m_idx = (m_idx + 1) % m_len;

            writePtr[s] = m_mix * m_buf[readIdx] + (1.0 - m_mix) * readPtr[s];
        }
    }

    void DelayModule::setMaxDelayInMs(double maxDelayInMs)
    {
        resizeBuffer(m_sampleRate, maxDelayInMs);
    }

    void DelayModule::setDelayInMs(double delayInMs)
    {
        m_delayInMs = std::min(delayInMs, m_maxDelayInMs);
        m_delayInSamples = static_cast<unsigned>(std::round(m_delayInMs / 1000.0 * m_sampleRate));
    }

    void DelayModule::setMix(double mix)
    {
        m_mix = std::min(1.0, std::max(0.0, mix));
    }

    void DelayModule::setFeedback(double fb)
    {
        m_fb = std::min(1.0, std::max(0.0, fb));
    }

    void DelayModule::setLowPass(bool active, double freq)
    {
        m_lpFilter.setBypass(active == false);
        m_lpFilter.setLPF(freq);
    };

    void DelayModule::setHighPass(bool active, double freq)
    {
        m_hpFilter.setBypass(active == false);
        m_hpFilter.setHPF(freq);
    };

    void DelayModule::resizeBuffer(double sampleRate, double maxDelayInMs)
    {
        unsigned maxDly = static_cast<unsigned>(std::ceil(maxDelayInMs / 1000.0 * sampleRate));

        if (maxDly > m_len)
        {
            m_buf.resize(maxDly, 0.0);
            m_len = maxDly;
        }
        m_sampleRate = sampleRate;
        m_maxDelayInSamples = maxDly;
        m_maxDelayInMs = maxDelayInMs;
    }
}