#pragma once

#include <jack/midiport.h>
#include <sndfile.h>
#include <string.h>
#include <samplerate.h>
#include <algorithm>


class Metronome {
    public:
        Metronome();
        ~Metronome();
        bool Init(unsigned buffersize, unsigned samplerate);
        void ProcessSysEx(jack_midi_event_t *event);
        void EndProcess();
        void GetClick(float* output);
        float GetTempo();

    private:
        bool m_initialized;
        float m_tempo;
        unsigned m_bufferSize;
        unsigned m_sampleRate;

        unsigned m_beat;
        unsigned m_numBeats;
        bool m_start;
        unsigned m_startIdx;
        unsigned m_sampleIdx;

        bool m_isRunning;
        bool m_print;
        bool m_clkSet;
        unsigned m_clkIdx;
        unsigned m_clkCount;
        unsigned m_clkLen;
        unsigned m_clkBuf;
        unsigned *m_clkBuffer;

        float *m_sampleBuffer;
        SNDFILE *m_clickHigh;
        SNDFILE *m_clickLow;
        unsigned m_clickHighCount;
        unsigned m_clickLowCount;

        float *m_clickHighInput;
        float *m_clickHighOutput;
        float *m_clickLowInput;
        float *m_clickLowOutput;
};