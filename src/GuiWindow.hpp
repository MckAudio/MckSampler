#pragma once

#include <string>
#include <thread>
#include <atomic>
#include <iostream>
#include <nlohmann/json.hpp>

#include "Types.hpp"

namespace webview
{
    class webview;
}
namespace httplib
{
    class Server;
}

class GuiWindow
{
public:
    GuiWindow();
    ~GuiWindow();
    bool Show(std::string title, std::string path, unsigned port = 9002);
    void Close();

    template <typename T>
    bool SendMessage(std::string section, std::string msgType, T &data)
    {
        if (m_isInitialized == false)
        {
            return false;
        }

        auto outMsg = MCK::Message();
        outMsg.section = section;
        outMsg.msgType = msgType;
        try
        {
            nlohmann::json j = outMsg;
            j["data"] = data;
            std::string out = "ReceiveMessage(" + j.dump() + ");";
            out = "1+2";
            return Evaluate(out);
        }
        catch (std::exception &e)
        {
            std::cerr << "Failed to convert message to JSON: " << e.what() << std::endl;
            return false;
        }
        return true;
    }

private:
    bool Evaluate(std::string msg);
    bool m_isInitialized;
    std::atomic<bool> m_done;
    httplib::Server *m_server;
    webview::webview *m_window;
    std::thread m_serverThread;
    std::thread m_windowThread;
};