#include "MainWindow.h"
#include "AboutDialog.h"
#include "MainApplication.h"
#include <fstream>

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

void MainWindow::startBackgroundProcess() {
    int child_pid;
    int std_err;
    std::vector<std::string> argumentVector;
    argumentVector.push_back("/usr/bin/pkexec");
    argumentVector.push_back(Utility::APPLICATION_PATH + "/Linux_CPU_Hotplugger");
    argumentVector.push_back("--min-core");
    argumentVector.push_back(std::to_string(static_cast<int>(this->minCores->get_value())));
    argumentVector.push_back("--max-core");
    argumentVector.push_back(std::to_string(static_cast<int>(this->maxCores->get_value())));
    argumentVector.push_back("--min-threshold");
    argumentVector.push_back(std::to_string(static_cast<float>(this->minThreshold->get_value())));
    argumentVector.push_back("--max-threshold");
    argumentVector.push_back(std::to_string(static_cast<float>(this->maxThreshold->get_value())));
    argumentVector.push_back("--sleep");
    argumentVector.push_back(std::to_string(static_cast<int>(this->intervals->get_value())));
    try {
        Glib::spawn_async_with_pipes(
                Utility::APPLICATION_PATH,
                argumentVector,
                Glib::SPAWN_DO_NOT_REAP_CHILD,
                sigc::slot0<void>(),
                &child_pid,
                nullptr,
                nullptr,
                &std_err
        );

        char charBuf;
        std::string error;
        while (read(std_err, &charBuf, 1) == 0) {
            error.push_back(charBuf);
        }

        if (! error.empty()) {
            // Prevent a zombie process
            kill(child_pid, SIGKILL);
            this->statusToggle->set_state(false);

            Gtk::MessageDialog errorDialog(*this, "Error occured", false, Gtk::MESSAGE_ERROR);
            errorDialog.set_secondary_text(error);
            errorDialog.run();
        }
        else {
            std::ofstream pidOut(this->BACKGROUND_RUN_PATH);
            pidOut << child_pid;
            pidOut.close();
        }
    }
    catch (Glib::SpawnError& ex) {
        std::cerr << ex.what() << std::endl;
    }
}

void MainWindow::setControlWidgetsState(bool state) {
    this->maxThreshold->set_editable(state);
    this->minThreshold->set_editable(state);
    this->minCores->set_editable(state);
    this->maxCores->set_editable(state);
    this->intervals->set_editable(state);
}

void MainWindow::toggleStatus() {
    if (this->statusToggle->property_active().get_value()) {
        this->startBackgroundProcess();

        // Disables the widgets
        this->setControlWidgetsState(false);
    }
    else {
        // Remove pid file
        Glib::RefPtr<Gio::File> pidFile = Gio::File::create_for_path(this->BACKGROUND_RUN_PATH);
        pidFile->remove();
        
        // Enables the widgets
        this->setControlWidgetsState(true);
    }
}