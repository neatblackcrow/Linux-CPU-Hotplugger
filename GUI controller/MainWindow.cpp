#include "MainWindow.h"
#include "AboutDialog.h"
#include "MainApplication.h"

MainWindow::MainWindow(BaseObjectType *cobject, const Glib::RefPtr<Gtk::Builder>& builder):
    Gtk::ApplicationWindow(cobject),
    builder(builder),
    statusLabel(nullptr), statusToggle(nullptr),
    minThreshold(nullptr), maxThreshold(nullptr),
    minCores(nullptr), maxCores(nullptr),
    intervals(nullptr) {

    this->set_size_request(500, 500);
    this->set_default_size(500, 500);

    // Add the action handlers
    this->add_action("exit", sigc::mem_fun0(*this, &MainWindow::quit));
    this->add_action("about", sigc::mem_fun0(*this, &MainWindow::showAbout));

    // Add the signal handlers
    this->builder->get_widget("toggleStatus", this->statusToggle);
    this->statusToggle->property_active().signal_changed().connect(sigc::mem_fun0(*this, &MainWindow::toggleStatus));

    // Just get the widgets
    this->builder->get_widget("labelStatus", this->statusLabel);
    this->builder->get_widget("spinMinCore", this->minCores);
    this->builder->get_widget("spinMaxCore", this->maxCores);
    this->builder->get_widget("spinMinThreshold", this->minThreshold);
    this->builder->get_widget("spinMaxThreshold", this->maxThreshold);
    this->builder->get_widget("spinInterval", this->intervals);

}

MainWindow::~MainWindow() {

}

void MainWindow::quit() {
    this->close();
}

void MainWindow::showAbout() {
    AboutDialog* aboutDialog = nullptr;
    this->builder->get_widget_derived("aboutDialog", aboutDialog);
    aboutDialog->set_transient_for(*this);
    aboutDialog->show();
}

void MainWindow::toggleStatus() {
    if (this->statusToggle->property_active().get_value()) {
        int child_pid;
        std::vector<std::string> argumentVector;
        argumentVector.push_back("/usr/bin/pkexec");
        argumentVector.push_back(Utility::APPLICATION_PATH + "/Linux_CPU_Hotplugger");
        argumentVector.push_back("--min-core " + std::to_string(static_cast<int>(this->minCores->get_value())));
        argumentVector.push_back("--max-core " + std::to_string(static_cast<int>(this->maxCores->get_value())));
        argumentVector.push_back("--min-threshold " + std::to_string(static_cast<float>(this->minThreshold->get_value())));
        argumentVector.push_back("--max-threshold " + std::to_string(static_cast<float>(this->maxThreshold->get_value())));
        argumentVector.push_back("--interval " + std::to_string(static_cast<int>(this->intervals->get_value())));
        try {
            Glib::spawn_async_with_pipes(
                    Utility::APPLICATION_PATH,
                    argumentVector,
                    Glib::SPAWN_DO_NOT_REAP_CHILD,
                    sigc::slot0<void>(),
                    &child_pid,
                    nullptr,
                    nullptr,
                    nullptr
            );
        }
        catch (Glib::SpawnError& ex) {
            std::cerr << ex.what() << std::endl;
        }
    }
    else {

    }
}