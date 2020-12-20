#include <jack/jack.h>
#include <jack/midiport.h>
#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
#include <vector>

#include <ncurses.h>

jack_client_t *client;
jack_port_t *input_port;

jack_nframes_t bufferSize;
jack_nframes_t sampleRate;

jack_nframes_t clkCount = 0;
jack_nframes_t clkLen = 24;
jack_nframes_t *clockBuffer;
jack_nframes_t clkIdx = 0;
long clkBuf = 0;
double tempo = 120.0;
bool isRunning = false;

WINDOW *window;
WINDOW *tempowin;

int xIdx = 0;
int xOff = 3;
int yIdx = 0;
int yOff = 2;

typedef struct
{
    jack_nframes_t time = 0;
    unsigned char chan = 0;
    unsigned char type = 0;
} midi_data;

int GetX()
{
    int x = xIdx + xOff;
    xIdx += 1;

    if (x >= COLS - xOff)
    {
        xIdx = 0;
    }

    return x;
}

int GetY()
{
    int y = yIdx + yOff;
    yIdx += 1;

    if (y >= LINES - yOff)
    {
        yIdx = 0;
    }

    return y;
}

static void signal_handler(int sig)
{
    // Save Connections
    const char** ports = jack_port_get_connections(input_port);

    if (ports != nullptr) {
        const char *ptr = ports[0];

        for (; ptr != nullptr; ptr += 1)
        {
            mvprintw(GetY(), xOff, "Connected to port %s\n", *ptr);
        }
    }

    jack_client_close(client);
    mvprintw(GetY(), xOff, "signal received, exiting ...\n");

    getch();

    endwin();
    exit(0);
}

static void usage()
{
}

static int process(jack_nframes_t nframes, void *arg)
{
    int i, j;
    void *input_buf = jack_port_get_buffer(input_port, nframes);
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
            tmp.type = (event.buffer[0] & 0xff);
            tmp.time = event.time;

            if (tmp.type == 0xf8)
            {
                if (clkSet)
                {
                    clockBuffer[clkIdx] = tmp.time - clkCount;
                }
                else
                {
                    clockBuffer[clkIdx] = clkBuf + tmp.time - clkCount;
                }
                clkSet = true;
                clkCount = tmp.time;

                clkIdx += 1;

                if (clkIdx >= clkLen)
                {
                    // Calculate Tempo
                    jack_nframes_t sum = 0;
                    for (j = 0; j < clkLen; j++)
                    {
                        sum += clockBuffer[j];
                    }
                    mvprintw(GetY(), xOff, "Sum: %d, SR: %d, BS: %d, ClkBuf: %d\n", sum, sampleRate, nframes, clkBuf);
                    tempo = 60.0 * (double)sampleRate / (double)sum;
                    mvprintw(GetY(), xOff, "Current Tempo is: %f BPM\n", tempo);
                    print = true;

                    clkIdx = 0;
                }
            }
            else if (tmp.type == 0xfa)
            {
                isRunning = true;
                clkSet = true;
                clkIdx = 0;
                clkCount = tmp.time;
            }
            else if (tmp.type == 0xfc)
            {
                isRunning = false;
            }
            else
            {
                mvprintw(GetY(), xOff, "SYS: Received system midi type %d / 0x%02X at index %d with values:\n", tmp.type, tmp.type, tmp.time);
                for (j = 1; j < event.size; j++)
                {
                    mvprintw(GetY(), xOff, " (%d): %d / 0x%02X\n", j, (event.buffer[j] & 0x7f), (event.buffer[j] & 0x7f));
                }
                print = true;
            }
        }
        else
        {
            tmp.type = (event.buffer[0] & 0xf0);
            tmp.chan = (event.buffer[0] & 0x0f);
            tmp.time = event.time;

            mvprintw(GetY(), xOff, "Chan #%d: Received midi type %d / 0x%02X at index %d with values:\n", tmp.chan + 1, tmp.type, tmp.type, tmp.time);
            for (j = 1; j < event.size; j++)
            {
                mvprintw(GetY(), xOff, " (%d): %d / 0x%02X\n", j, (event.buffer[j] & 0x7f), (event.buffer[j] & 0x7f));
            }
            print = true;
        }
    }

    if (isRunning)
    {
        if (clkSet == false)
        {
            clkBuf = clkBuf + nframes;
        }
        else
        {
            clkBuf = nframes;
        }
    }
    if (print)
    {
        mvwprintw(tempowin, 1, 1, "%.2f BPM", tempo);
        refresh();
        wrefresh(tempowin);
    }

    return 0;
}

int main(int narg, char **args)
{
    int i;

    // Curses
    window = initscr();
    clear();
    noecho();
    curs_set(0);

    start_color();
    init_pair(1, COLOR_WHITE, COLOR_BLUE);
    init_pair(2, COLOR_BLUE, COLOR_WHITE);

    // tempowin
    tempowin = newwin(3, 12, yOff, COLS - xOff - 12);
    bkgd(COLOR_PAIR(1));
    wbkgd(tempowin, COLOR_PAIR(2));

    if ((client = jack_client_open("MidiMonitor", JackNullOption, NULL)) == 0)
    {
        fprintf(stderr, "JACK server not running?\n");
        return 1;
    }
    jack_set_process_callback(client, process, 0);

    input_port = jack_port_register(client, "in", JACK_DEFAULT_MIDI_TYPE, JackPortIsInput, 0);
    bufferSize = jack_get_buffer_size(client);
    sampleRate = jack_get_sample_rate(client);

    clockBuffer = new jack_nframes_t(clkLen);

    if (jack_activate(client))
    {
        fprintf(stderr, "cannot activate client");
        endwin();
        return 1;
    }

    /* install a signal handler to properly quits jack client */
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
    endwin();

    delete clockBuffer;
    exit(0);
}
