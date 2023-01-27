#include "OnePoleFilter.hpp"

#include <cmath>
#include <cstddef>
#include <algorithm>

namespace MckDsp
{

    void OnePoleFilter::prepareToPlay(double sampleRate, int samplesPerBlock)
    {
        m_sampleRate = sampleRate;
    }

    double OnePoleFilter::processSample(double in)
    {
        m_histOut = in * m_a0 + m_histIn * m_a1 + m_histOut * m_b1;
        m_histIn = in;
        return m_bypass ? in : m_histOut;
    }
    
    void OnePoleFilter::setLPF(double freq)
    {
        freq = std::max(10.0, std::min(20000.0, freq));
        freq = freq * 2.0 * M_PI;

        double w = 2.0 * m_sampleRate;
        double n = 1.0 / (freq + w);
        m_a0 = m_a1 = freq * n;
        m_b1 = (w - freq) * n;
    }

    void OnePoleFilter::setHPF(double freq)
    {
        freq = std::max(10.0, std::min(20000.0, freq));
        freq = freq * 2.0 * M_PI;

        double w = 2.0 * m_sampleRate;
        double n = 1.0 / (freq + w);
        m_a0 = w * n;
        m_a1 = -m_a0;
        m_b1 = (w - freq) * n;
    }

    void OnePoleFilter::setBypass(bool bypass)
    {
        m_histIn = 0.0;
        m_histOut = 0.0;
        m_bypass = bypass;
    }
}