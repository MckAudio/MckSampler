#include "../webview/webview.h"
#include "../cpp-httplib/httplib.h"
#include <thread>
#include <iostream>
#include <gtk/gtk.h>

void SendMessage(std::string msg, std::string cmd, void* arg)
{

    std::cout << msg << " - " << cmd << std::endl;
    webview::webview *w = (webview::webview *)arg;
    w->eval("ReceiveMessage(\"Hallo\")");
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
    GtkWindow *window = (GtkWindow *)w.window();
    auto t = std::thread([window](){
        GtkWidget *dialog;
        GtkDialogFlags flags = GTK_DIALOG_DESTROY_WITH_PARENT;
        unsigned idx = 0;
        while(true) {
            sleep(5);
            dialog = gtk_message_dialog_new(window, flags, GTK_MESSAGE_INFO, GTK_BUTTONS_OK_CANCEL, "This is dialog #%d!", idx + 1);
            gtk_dialog_run(GTK_DIALOG(dialog));
            gtk_widget_destroy(dialog);
            idx += 1;
        };
    });
    w.run();
    w.terminate();
    svr.stop();
    httpThread.join();
    return 0;
}