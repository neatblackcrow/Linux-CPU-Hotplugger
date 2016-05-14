#include <gtkmm.h>
#include "MainApplication.h"
#include "MainWindow.h"

namespace Utility {
    std::string APPLICATION_PATH;
    void initUtil() {
        GError *readLinkError;
        APPLICATION_PATH = g_file_read_link("/proc/self/exe", &readLinkError);
        APPLICATION_PATH.replace(APPLICATION_PATH.find_last_of("/"),
                                          APPLICATION_PATH.length(), "");
        if (APPLICATION_PATH.empty()) {
            std::cerr << readLinkError->message << std::endl;
        }
    }
};


int main(int argc, char* argv[]) {
    Utility::initUtil();
    Glib::RefPtr<Gtk::Application> application = Gtk::Application::create(argc, argv, "org.fieldfirst.CPUHotplugger");
    Glib::RefPtr<Gtk::Builder> builder = Gtk::Builder::create();
    MainWindow* mainWindow = nullptr;
    try {
        builder->add_from_file(Utility::APPLICATION_PATH + "/LinuxCPUHotplugger.glade");
        builder->get_widget_derived("mainWindow", mainWindow);
    }
    catch (Glib::FileError& ex) {
        std::cerr << ex.what() << std::endl;    // File's errors
    }
    catch (Gtk::BuilderError& ex) {
        std::cerr << ex.what() << std::endl;    // Internal builder's errors e.g. parsing error
    }
    catch (Glib::Error& ex) {
        std::cerr << ex.what() << std::endl;    // Other errors
    }
    return application->run(*mainWindow);
}