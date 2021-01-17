#pragma once

#include <string>
#include <thread>
#include <atomic>
#include <iostream>
#include <nlohmann/json.hpp>
#include <concurrentqueue.h>
#include <condition_variable>
#include <mutex>
#include "Types.hpp"

namespace webview
{
    class webview;
}
namespace httplib
{
    class Server;
}
namespace mck
{
    class Processing;
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
            return Evaluate(out);
        }
        catch (std::exception &e)
        {
            std::cerr << "Failed to convert message to JSON: " << e.what() << std::endl;
            return false;
        }
        return true;
    }
    void ReceiveMessage(std::string msg);
    void SetProcessingPtr(mck::Processing *proc);

private:
    bool Evaluate(std::string msg);
    void SendThread();

    mck::Processing *m_proc;

    bool m_isInitialized;
    std::atomic<bool> m_done;
    httplib::Server *m_server;
    webview::webview *m_window;
    std::thread m_serverThread;
    std::thread m_windowThread;
    std::thread m_sendThread;

    moodycamel::ConcurrentQueue<std::string> m_sendQueue;
    std::mutex m_sendMutex;
    std::condition_variable m_sendCond;
};