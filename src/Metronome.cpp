#include "Metronome.hpp"

Metronome::Metronome() : m_tempo(120.0f), m_initialized(false), m_isRunning(false), m_clkSet(false), m_clkLen(24), m_clkCount(0), m_clkIdx(0), m_clkBuf(0), m_print(false), m_beat(3), m_numBeats(4), m_sampleIdx(0), m_startIdx(0), m_start(false)
{
    m_clkBuffer = new unsigned[m_clkLen];
}

Metronome::~Metronome()
{
    delete m_clkBuffer;
    if (m_initialized)
    {
        delete m_sampleBuffer;
    }
}

bool Metronome::Init(unsigned buffersize, unsigned samplerate)
{
    if (m_initialized)
    {
        return false;
    }

    m_bufferSize = buffersize;
    m_sampleRate = samplerate;

    // Read Wav Files
    SF_INFO info;
    m_clickHigh = sf_open("../content/metro_1.wav", SFM_READ, &info);
    m_clickHighCount = info.frames;
    // Sample rate Conversion
    if (info.samplerate != samplerate)
    {
        m_clickHighInput = new float[m_clickHighCount];
        m_clickHighOutput = new float[m_clickHighCount];
        sf_read_float(m_clickHigh, m_clickHighInput, m_clickHighCount);
        SRC_DATA src;
        src.data_in = m_clickHighInput;
        src.data_out = m_clickHighOutput;
        src.input_frames = m_clickHighCount;
        src.output_frames = m_clickHighCount;
        src.src_ratio = (double)samplerate / (double)info.samplerate;
        src_simple(&src, SRC_SINC_BEST_QUALITY, 1);
        //delete m_clickHighInput;
    }
    else
    {
        m_clickHighOutput = new float[m_clickHighCount];
        sf_read_float(m_clickHigh, m_clickHighOutput, m_clickHighCount);
    }

    m_clickLow = sf_open("../content/metro_2.wav", SFM_READ, &info);
    m_clickLowCount = info.frames;
    // Sample rate Conversion
    if (info.samplerate != samplerate)
    {
        m_clickLowInput = new float[m_clickLowCount];
        m_clickLowOutput = new float[m_clickLowCount];
        sf_read_float(m_clickLow, m_clickLowInput, m_clickLowCount);
        SRC_DATA src;
        src.data_in = m_clickLowInput;
        src.data_out = m_clickLowOutput;
        src.input_frames = m_clickLowCount;
        src.output_frames = m_clickLowCount;
        src.src_ratio = (double)samplerate / (double)info.samplerate;
        src_simple(&src, SRC_SINC_BEST_QUALITY, 1);
        //delete m_clickLowInput;
    }
    else
    {
        m_clickLowOutput = new float[m_clickLowCount];
        sf_read_float(m_clickLow, m_clickLowOutput, m_clickLowCount);
    }

    m_sampleBuffer = new float[m_bufferSize];

    m_initialized = true;

    return true;
}

void Metronome::ProcessSysEx(jack_midi_event_t *event)
{
    if (m_initialized == false)
    {
        return;
    }

    unsigned char type = (event->buffer[0] & 0xff);

    if (type == 0xf8)
    {
        if (m_clkSet)
        {
            m_clkBuffer[m_clkIdx] = event->time - m_clkCount;
        }
        else
        {
            m_clkBuffer[m_clkIdx] = m_clkBuf + event->time - m_clkCount;
        }
        m_clkSet = true;
        m_clkCount = event->time;

        if (m_clkIdx == 0)
        {
            m_startIdx = event->time;
            m_start = true;
        }

        m_clkIdx += 1;

        if (m_clkIdx >= m_clkLen)
        {
            // Calculate Tempo
            unsigned sum = 0;
            for (unsigned j = 0; j < m_clkLen; j++)
            {
                sum += m_clkBuffer[j];
            }
            printf("Sum: %d, SR: %d, BS: %d, ClkBuf: %d\n", sum, m_sampleRate, m_bufferSize, m_clkBuf);
            m_tempo = 60.0 * (double)m_sampleRate / (double)sum;
            printf("Current Tempo is: %f BPM\n", m_tempo);
            m_print = true;

            m_clkIdx = 0;
        }
    }
    else if (type == 0xfa)
    {
        m_isRunning = true;
        m_clkSet = true;
        m_clkIdx = 0;
        m_beat = 3;
        m_clkCount = event->time;
    }
    else if (type == 0xfb)
    {
        m_isRunning = true;
    }
    else if (type == 0xfc)
    {
        m_isRunning = false;
    }
}

void Metronome::GetClick(float *out)
{
    /*
    if (m_isRunning) {
        memset(m_sampleBuffer, 0, m_bufferSize * sizeof(float));
        if (m_start) {
            if (m_beat == 0) {
                sf_seek(m_clickHigh, 0, SEEK_SET);
                m_sampleIdx = sf_read_float(m_clickHigh, m_sampleBuffer, m_bufferSize - m_startIdx);
            } else {
                sf_seek(m_clickLow, 0, SEEK_SET);
                m_sampleIdx = sf_read_float(m_clickLow, m_sampleBuffer, m_bufferSize - m_startIdx);
            }
            memcpy(out + m_startIdx, m_sampleBuffer, (m_bufferSize - m_startIdx) * sizeof(float));
            m_start = false;
        } else {
            unsigned len = 0;
            if (m_beat == 0) 
            {
                len = sf_read_float(m_clickHigh, m_sampleBuffer, m_bufferSize);
            } else {
                len = sf_read_float(m_clickLow, m_sampleBuffer, m_bufferSize);
            }
            m_sampleIdx += len;
            memcpy(out, m_sampleBuffer, m_bufferSize * sizeof(float));
        }
    }
    */

    if (m_isRunning)
    {
        if (m_beat == 0)
        {
            unsigned len = std::min(m_bufferSize, (m_clickHighCount - m_sampleIdx));
            if (m_start)
            {
                len = std::min(len, m_startIdx);
            }
            memcpy(out, m_clickHighOutput + m_sampleIdx, len * sizeof(float));
            m_sampleIdx += len;
        }
        else
        {
            unsigned len = std::min(m_bufferSize, (m_clickLowCount - m_sampleIdx));
            if (m_start)
            {
                len = std::min(len, m_startIdx);
            }
            memcpy(out, m_clickLowOutput + m_sampleIdx, len * sizeof(float));
            m_sampleIdx += len;
        }

        if (m_start)
        {
            m_beat = (m_beat + 1) % m_numBeats;
            printf("Beat %d/%d\n", m_beat, m_numBeats);
            m_sampleIdx = 0;
            unsigned len = m_bufferSize - m_startIdx;
            if (m_beat == 0)
            {
                len = std::min(m_clickHighCount, len);
                memcpy(out + m_startIdx, m_clickHighOutput, len * sizeof(float));
            }
            else
            {
                len = std::min(m_clickLowCount, len);
                memcpy(out + m_startIdx, m_clickLowOutput, len* sizeof(float));
            }
            m_sampleIdx = len;
            m_start = false;
        }
    }
}

void Metronome::EndProcess()
{

    if (m_isRunning)
    {
        if (m_clkSet == false)
        {
            m_clkBuf += m_bufferSize;
        }
        else
        {
            m_clkBuf = m_bufferSize;
        }
    }
    m_clkSet = false;
}