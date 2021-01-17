#include "GuiWindow.hpp"
#include "Processing.hpp"
#include "webview/webview.h"
#include "cpp-httplib/httplib.h"
#include "Types.hpp"

void MsgFromGui(std::string idx, std::string msg, void *arg)
{
    GuiWindow *win = (GuiWindow *)arg;
    std::cout << "Msg from GUI: " << idx << " : " << msg << std::endl;

    win->ReceiveMessage(msg);

    return;
}

GuiWindow::GuiWindow()
    : m_isInitialized(false),m_done(false)
{
    m_server = new httplib::Server();
    m_window = new webview::webview(true, nullptr);
    m_sendQueue = moodycamel::ConcurrentQueue<std::string>(512);
}

GuiWindow::~GuiWindow()
{
    if (m_isInitialized)
    {
        Close();
    }
    delete m_server;
    delete m_window;
}

bool GuiWindow::Show(std::string title, std::string path, unsigned port)
{
    if (m_isInitialized == true)
    {
        std::cerr << "GuiWindow is already shown" << std::endl;
        return false;
    }

    // HTML Server
    bool ret = m_server->set_mount_point("/", path.c_str());
    if (ret == false)
    {
        std::cerr << "Failed to set mount point to path " << path << "! Does the path exist?" << std::endl;
        return false;
    }
    m_serverThread = std::thread([this, &port]() {
        m_server->listen("localhost", port);
    });

    // Send Thread
    //m_sendThread = std::thread(&GuiWindow::SendThread, this);

    m_window->set_title(title);
    m_window->set_size(1280, 720, WEBVIEW_HINT_NONE);
    m_window->navigate("http://localhost:" + std::to_string(port));
    m_window->bind("SendMessage", MsgFromGui, (void *)this);

    m_isInitialized = true;
    m_window->run();

    return true;
}

void GuiWindow::Close()
{
    m_done = true;
    if (m_windowThread.joinable())
    {
        m_window->terminate();
        m_windowThread.join();
    }
    if (m_serverThread.joinable())
    {
        m_server->stop();
        m_serverThread.join();
    }
    if (m_sendThread.joinable())
    {
        m_sendThread.join();
    }
    m_isInitialized = false;
}
void GuiWindow::ReceiveMessage(std::string msg)
{
    std::vector<MCK::Message> mckMsgs;
    try
    {
        nlohmann::json j = nlohmann::json::parse(msg);
        mckMsgs = j.get<std::vector<MCK::Message>>();
    }
    catch (std::exception &e)
    {
        return;
    }
    if (m_proc != nullptr)
    {
        for(auto &m : mckMsgs) {
            m_proc->ReceiveMessage(m);
        }
    }
}
void GuiWindow::SetProcessingPtr(mck::Processing *proc)
{
    m_proc = proc;
}

bool GuiWindow::Evaluate(std::string msg)
{
    if (m_isInitialized == false)
    {
        return false;
    }

    m_window->dispatch([&, msg]() {
        m_window->eval(msg);
    });

    return true;

    //return m_sendQueue.try_enqueue(msg);
}

void GuiWindow::SendThread()
{
    while (true)
    {
        if (m_done.load())
        {
            break;
        }
        std::string msg = "";
        while (m_sendQueue.try_dequeue(msg))
        {
            m_window->dispatch([&]() {
                m_window->eval(msg);
            });
            //m_window->eval(msg);
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
}