#include "../webview/webview.h"
#include "../cpp-httplib/httplib.h"
#include <thread>
#include <iostream>

void SendMessage(std::string msg, std::string cmd, void* arg)
{

    std::cout << msg << " - " << cmd << std::endl;
    webview::webview *w = (webview::webview *)arg;
    w->eval("RecvMsg(\"Hallo\")");
}

int main() {
    httplib::Server svr;

    auto ret = svr.set_mount_point("/", "./www");
    if (ret == false) {
        return 1;
    }

    auto httpThread = std::thread([&svr]() {
        svr.listen("localhost", 9002);
    });

    webview::webview w(true, nullptr);
    w.set_title("Webview Example");
    w.set_size(480,320, WEBVIEW_HINT_NONE);
    w.navigate("http://localhost:9002");

    w.bind("SendMessage", SendMessage, (void *)&w);
    w.run();
    w.terminate();
    svr.stop();
    httpThread.join();
    return 0;
}