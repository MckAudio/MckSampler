/*
    Copyright (C) 2004 Ian Esten
    
    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#include <jack/jack.h>
#include <jack/midiport.h>
#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
#include <vector>
#include "Metronome.hpp"

jack_client_t *client;
jack_port_t *input_port;
jack_port_t *output_port;
jack_port_t *click_port;

jack_nframes_t bufferSize;
jack_nframes_t sampleRate;
jack_nframes_t *samples;

jack_nframes_t recIdx = 0;
jack_nframes_t playIdx = 0;
bool isRecording = false;
bool isPlaying = false;

unsigned char recChan = 2;
unsigned char ctrlChan = 10;

typedef struct
{
    jack_nframes_t idx = 0;
    jack_nframes_t time = 0;
    unsigned char chan = 0;
    unsigned char type = 0;
    unsigned char freq = 0;
    unsigned char velo = 0;
} midi_data;

std::vector<midi_data> recording;
jack_nframes_t recordingIdx = 0;

Metronome m_metro;

static void signal_handler(int sig)
{
    jack_client_close(client);
    fprintf(stderr, "signal received, exiting ...\n");
    exit(0);
}

static void usage()
{
}

static int process(jack_nframes_t nframes, void *arg)
{
    jack_nframes_t idx;
    void *input_buf = jack_port_get_buffer(input_port, nframes);
    void *output_buf = jack_port_get_buffer(output_port, nframes);
    void *click_buf = jack_port_get_buffer(click_port, nframes);
    memset(click_buf, 0, nframes * sizeof(jack_default_audio_sample_t));

    unsigned char *buffer;
    jack_midi_clear_buffer(output_buf);

    jack_midi_event_t event;
    jack_nframes_t eventIdx = 0;
    jack_nframes_t eventCount = jack_midi_get_event_count(input_buf);
    bool sysMsg = false;
    midi_data tmp;

    for (unsigned i = 0; i < eventCount; i++)
    {
        jack_midi_event_get(&event, input_buf, i);

        sysMsg = (event.buffer[0] & 0xf0) == 0xf0;

        if (sysMsg)
        {
            m_metro.ProcessSysEx(&event);
        }
        else if (isRecording)
        {
            tmp.type = (event.buffer[0] & 0xf0);
            tmp.chan = (event.buffer[0] & 0x0f);

            if (tmp.chan == ctrlChan && tmp.type == 0x80)
            {
                tmp.freq = (event.buffer[1] & 0x7f);
                if (tmp.freq == 21)
                {
                    printf("Recording stopped!\n");
                    isRecording = false;
                }
            }
            else if (tmp.chan == recChan)
            {
                // Note On or Note Off
                if (tmp.type == 0x80 || tmp.type == 0x90)
                {
                    tmp.time = event.time;
                    tmp.idx = recIdx + event.time;
                    tmp.freq = (event.buffer[1] & 0x7f);
                    tmp.velo = (event.buffer[2] & 0x7f);

                    if (tmp.type == 0x80)
                    {
                        printf("Recorded a new note on channel %d with freq %d and velo %d at idx %d\n", tmp.chan, tmp.freq, tmp.velo, tmp.idx);
                    }

                    recording.push_back(tmp);
                }
            }
        }
        else if (isPlaying)
        {
            jack_midi_event_t event;
            jack_nframes_t eventIdx = 0;
            jack_nframes_t eventCount = jack_midi_get_event_count(input_buf);

            for (idx = 0; idx < eventCount; idx++)
            {
                jack_midi_event_get(&event, input_buf, idx);

                tmp.type = (event.buffer[0] & 0xf0);
                tmp.chan = (event.buffer[0] & 0x0f);

                if (tmp.chan == ctrlChan && tmp.type == 0x80)
                {
                    tmp.freq = (event.buffer[1] & 0x7f);

                    if (tmp.freq == 21)
                    {
                        printf("Stopped!\n");
                        isRecording = false;
                        isPlaying = false;
                        recIdx = 0;
                        playIdx = 0;
                        recordingIdx = 0;
                    }
                }
            }
        }
        else
        {

            tmp.type = (event.buffer[0] & 0xf0);
            tmp.chan = (event.buffer[0] & 0x0f);

            if (tmp.chan == ctrlChan && tmp.type == 0x80)
            {
                tmp.freq = (event.buffer[1] & 0x7f);

                if (tmp.freq == 21)
                {
                    printf("Stopped!\n");
                    isRecording = false;
                    isPlaying = false;
                    recIdx = 0;
                    playIdx = 0;
                    recordingIdx = 0;
                }
                else if (tmp.freq == 22)
                {
                    printf("Playback started!\n");
                    isRecording = false;
                    isPlaying = true;
                    playIdx = 0;
                    recordingIdx = 0;
                }
                else if (tmp.freq == 23)
                {
                    printf("Recording started!\n");
                    isPlaying = false;
                    isRecording = true;
                    recIdx = 0;
                    recordingIdx = 0;
                    recording.clear();
                }
            }
        }
    }

    if (isRecording)
    {
        recIdx += bufferSize;
    }
    else if (isPlaying)
    {
        for (idx = 0; idx < nframes; idx++)
        {
            while (isPlaying)
            {
                if (recordingIdx >= recording.size())
                {
                    isPlaying = false;
                    printf("Playback stopped!\n");
                    break;
                }
                tmp = recording[recordingIdx];

                if (tmp.idx == playIdx + idx)
                {
                    printf("Playing a new note on channel %d with freq %d and velo %d at idx %d\n", tmp.chan, tmp.freq, tmp.velo, tmp.idx);

                    // Play out note
                    buffer = jack_midi_event_reserve(output_buf, idx, 3);
                    /*				printf("wrote a note on, port buffer = 0x%x, event buffer = 0x%x\n", port_buf, buffer);*/
                    buffer[2] = tmp.velo; /* velocity */
                    buffer[1] = tmp.freq;
                    buffer[0] = tmp.type + tmp.chan; /* note on */
                }
                else if (tmp.idx > playIdx + idx)
                {
                    break;
                }

                recordingIdx += 1;
            }
        }

        playIdx += bufferSize;
    }

    m_metro.GetClick((float *)click_buf);

    m_metro.EndProcess();

    return 0;
}

int main(int narg, char **args)
{
    int i;

    if ((client = jack_client_open("MidiLooper", JackNullOption, NULL)) == 0)
    {
        fprintf(stderr, "JACK server not running?\n");
        return 1;
    }
    jack_set_process_callback(client, process, 0);

    input_port = jack_port_register(client, "in", JACK_DEFAULT_MIDI_TYPE, JackPortIsInput, 0);
    output_port = jack_port_register(client, "out", JACK_DEFAULT_MIDI_TYPE, JackPortIsOutput, 0);
    click_port = jack_port_register(client, "click", JACK_DEFAULT_AUDIO_TYPE, JackPortIsOutput, 0);

    bufferSize = jack_get_buffer_size(client);
    sampleRate = jack_get_sample_rate(client);

    m_metro.Init(bufferSize, sampleRate);

    recording.reserve(5 * sampleRate);

    if (jack_activate(client))
    {
        fprintf(stderr, "cannot activate client");
        return 1;
    }

#ifndef WIN32
    signal(SIGQUIT, signal_handler);
    signal(SIGHUP, signal_handler);
#endif
    signal(SIGTERM, signal_handler);
    signal(SIGINT, signal_handler);

#ifdef WIN32
    Sleep(-1);
#else
    sleep(-1);
#endif

    jack_client_close(client);
    exit(0);
}
