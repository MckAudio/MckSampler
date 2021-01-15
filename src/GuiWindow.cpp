#include "GuiWindow.hpp"
#include "Processing.hpp"
#include "webview/webview.h"
#include "cpp-httplib/httplib.h"
#include "Types.hpp"

void GetData(std::string idx, std::string msg, void *arg)
{
    GuiWindow *win = (GuiWindow *)arg;
    std::cout << "GetData: " << idx << " : " << msg << std::endl;

    win->SetIsOpen(true);

    MCK::PadData pd;
    pd.index = 42;

    win->SendMessage(std::string("hallo"), std::string("hey"), pd);

    return;
}
void MsgFromGui(std::string idx, std::string msg, void *arg)
{
    GuiWindow *win = (GuiWindow *)arg;
    std::cout << "Msg from GUI: " << idx << " : " << msg << std::endl;

    win->ReceiveMessage(msg);

    return;
}

GuiWindow::GuiWindow()
    : m_isInitialized(false), m_isOpen(false), m_done(false)
{
    m_server = new httplib::Server();
    m_window = new webview::webview(true, nullptr);
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

    m_window->set_title(title);
    m_window->set_size(1280, 720, WEBVIEW_HINT_NONE);
    m_window->navigate("http://localhost:" + std::to_string(port));
    m_window->bind("GetData", GetData, (void *)this);
    m_window->bind("SendMessage", MsgFromGui, (void *)this);

    m_isInitialized = true;
    //m_windowThread = std::thread([this]() {
    m_window->run();
    //});

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
    m_isInitialized = false;
}
void GuiWindow::ReceiveMessage(std::string msg)
{
    if (m_proc != nullptr)
    {
        auto mckmsg = MCK::Message();
        mckmsg.section = "trigger";
        m_proc->ReceiveMessage(mckmsg);
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
    if (m_isOpen == false)
    {
        return false;
    }
    try
    {
        m_window->eval(msg);
    }
    catch (std::exception &e)
    {
        std::cerr << "GUI window is not shown yet: " << e.what() << std::endl;
    }
    return true;
}
