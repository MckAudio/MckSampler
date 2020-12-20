#include <jack/jack.h>
#include <jack/midiport.h>
#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
#include <vector>
#include <string.h>

#include "Metronome.hpp"

jack_client_t *client;
jack_port_t *input_port;
jack_port_t *output_port;

jack_nframes_t bufferSize;
jack_nframes_t sampleRate;

jack_nframes_t clkCount = 0;
jack_nframes_t clkLen = 24;
jack_nframes_t *clockBuffer;
jack_nframes_t clkIdx = 0;
long clkBuf = 0;
double tempo = 120.0;
bool isRunning = false;

int xIdx = 0;
int xOff = 3;
int yIdx = 0;
int yOff = 2;

Metronome m_metro;

typedef struct
{
    jack_nframes_t time = 0;
    unsigned char chan = 0;
    unsigned char type = 0;
} midi_data;


static void signal_handler(int sig)
{
    jack_client_close(client);
    printf("signal received, exiting ...\n");

    exit(0);
}

static void usage()
{
}

static int process(jack_nframes_t nframes, void *arg)
{
    int i, j;
    void *input_buf = jack_port_get_buffer(input_port, nframes);
    void *output_buf = jack_port_get_buffer(output_port, nframes);
    memset(output_buf, 0, nframes * sizeof(jack_default_audio_sample_t));

    unsigned char *buffer;

    // Monitor
    jack_midi_event_t event;
    jack_nframes_t eventIdx = 0;
    jack_nframes_t eventCount = jack_midi_get_event_count(input_buf);
    midi_data tmp;
    bool sysMsg = false;
    bool clkSet = false;
    bool print = false;


    for (i = 0; i < eventCount; i++)
    {
        jack_midi_event_get(&event, input_buf, i);

        sysMsg = (event.buffer[0] & 0xf0) == 0xf0;

        if (sysMsg)
        {
            m_metro.ProcessSysEx(&event);
        }
    }

    m_metro.GetClick((float *) output_buf);

    m_metro.EndProcess();

    return 0;
}

int main(int narg, char **args)
{
    int i;

    if ((client = jack_client_open("MidiMetronome", JackNullOption, NULL)) == 0)
    {
        fprintf(stderr, "JACK server not running?\n");
        return 1;
    }
    jack_set_process_callback(client, process, 0);

    input_port = jack_port_register(client, "in", JACK_DEFAULT_MIDI_TYPE, JackPortIsInput, 0);
    output_port = jack_port_register(client, "click", JACK_DEFAULT_AUDIO_TYPE, JackPortIsOutput, 0);
    bufferSize = jack_get_buffer_size(client);
    sampleRate = jack_get_sample_rate(client);

    clockBuffer = new jack_nframes_t(clkLen);

    m_metro.Init(bufferSize, sampleRate);

    if (jack_activate(client))
    {
        fprintf(stderr, "cannot activate client");
        return 1;
    }

    /* install a signal handler to properly quit jack client */
#ifndef WIN32
    signal(SIGQUIT, signal_handler);
    signal(SIGHUP, signal_handler);
#endif
    signal(SIGTERM, signal_handler);
    signal(SIGINT, signal_handler);

    /* run until interrupted */
#ifdef WIN32
    Sleep(-1);
#else
    sleep(-1);
#endif

    jack_client_close(client);

    delete clockBuffer;
    exit(0);
}
