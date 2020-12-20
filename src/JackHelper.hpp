#pragma once

#include <jack/jack.h>
#include <jack/midiport.h>

#include "Types.hpp"

namespace MCK {
    namespace JACK {
        bool GetConnections(jack_client_t *client, jack_port_t *port, std::vector<Connection> &connections);
        bool SetConnections(jack_client_t *client, jack_port_t *port, std::vector<Connection> &connections, bool isInput);
    }
}