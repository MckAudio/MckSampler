#include <cstdio>   // fprintf
#include <unistd.h> // sleep
#include <stdlib.h>
#include <signal.h>
#include <string>
#include <string.h>
#include <iostream>

// GUI
#include <GuiWindow.hpp>

// OWN Header
#include "Config.hpp"
#include <MckHelper/JackHelper.hpp>
#include <MckHelper/Transport.hpp>
#include "Processing.hpp"

// APP
std::atomic<bool> m_done = false;

// GUI
mck::GuiWindow m_gui;

// PROCESSING
mck::Processing m_processing;

void CloseApplication(bool saveConnections = true)
{
    m_done = true;
    m_processing.Close();
    m_gui.Close();
    exit(0);
}

static void SignalHandler(int sig)
{
    std::fprintf(stdout, "Signal %d received, exiting...\n", sig);
    CloseApplication();
}

int main(int argc, char **argv)
{
    // Get Arguments
    if (argc >= 2)
    {
        std::string command(argv[1]);
        if (command == "--help" || command == "-h")
        {
            printf("MckSampler by Matt McK\nUsage:\n\tmck-sampler [OPTION]\nOptions:\n");
            printf("\t--help, -h\tShow this help\n");
            exit(0);
        }
    }

    // Init Processing
    bool ret = m_processing.Init();
    if (ret == false)
    {
        return EXIT_FAILURE;
    }

    m_processing.SetGuiPtr(&m_gui);
    m_gui.SetBasePtr((mck::GuiBase *)&m_processing);

    signal(SIGQUIT, SignalHandler);
    signal(SIGTERM, SignalHandler);
    signal(SIGHUP, SignalHandler);
    signal(SIGINT, SignalHandler);

    mck::GuiSettings guiSettings;
    guiSettings.height = 480;
    guiSettings.width = 800;
    guiSettings.title = "MckSampler";
#ifdef DEBUG
    guiSettings.path = "./www";
    guiSettings.port = 3000;
    m_gui.ShowDebug(guiSettings);
#else
    guiSettings.path = "/usr/share/mck-sampler/gui";
    guiSettings.port = 9002;
    m_gui.Show(guiSettings);
#endif

    CloseApplication();

    return EXIT_SUCCESS;
}
