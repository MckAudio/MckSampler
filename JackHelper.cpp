#include "JackHelper.h"

bool MCK::JACK::GetConnections(jack_client_t *client, jack_port_t *port, std::vector<Connection> &connections)
{
    connections.clear();

    if (client == nullptr || port == nullptr)
    {
        return false;
    }
    const char **cons = jack_port_get_all_connections(client, port);
    if (cons)
    {
        MCK::Connection connection;
        const char **con = cons;
        for (; *con; con++)
        {
            connection.name = std::string(*con);
            connections.push_back(connection);
        }
        jack_free(cons);
    }

    return true;
}

bool MCK::JACK::SetConnections(jack_client_t *client, jack_port_t *port, std::vector<Connection> &connections, bool isInput)
{
    if (client == nullptr || port == nullptr)
    {
        return false;
    }
    if (jack_port_connected(port) > 0)
    {
        jack_port_disconnect(client, port);
    }
    const char* name = jack_port_name(port);
    bool ret = true;

    for (auto &c : connections)
    {
        if (isInput) {
            ret &= (jack_connect(client, c.name.c_str(), name) == 0);
        } else {
            ret &= (jack_connect(client, name, c.name.c_str()) == 0);
        }
    }

    return ret;
}