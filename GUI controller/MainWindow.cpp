#include "MainWindow.h"
#include "AboutDialog.h"
#include "MainApplication.h"
#include <fcntl.h>

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

    // Just get the widgets
    this->builder->get_widget("labelStatus", this->statusLabel);
    this->builder->get_widget("toggleStatus", this->statusToggle);
    this->builder->get_widget("spinMinCore", this->minCores);
    this->builder->get_widget("spinMaxCore", this->maxCores);
    this->builder->get_widget("spinMinThreshold", this->minThreshold);
    this->builder->get_widget("spinMaxThreshold", this->maxThreshold);
    this->builder->get_widget("spinInterval", this->intervals);

    // Initialize the Gio and setup the widgets
    // Note: statusToggle (Gtk::Switch) must be set before attach its signal to slot
    Gio::init();
    this->widgetsSetup();

    // Add the signal handlers
    this->statusToggle->property_state().signal_changed().connect(sigc::mem_fun0(*this, &MainWindow::toggleStatus));

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

void MainWindow::widgetsSetup() {
    // Check if the pid file exists (backend already running)
    Glib::RefPtr<Gio::File> pidFile = Gio::File::create_for_path(this->BACKGROUND_RUN_PATH);
    if (pidFile->query_exists()) {
        // Diable the widgets
        this->statusToggle->set_state(true);
        this->setControlWidgetsState(false);
        this->statusLabel->set_label("Hotplugger is <b>enabled</b>");
    }
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

        int stdErrFlag = fcntl(std_err, F_GETFL, 0);
        fcntl(std_err, F_SETFL, stdErrFlag | O_NONBLOCK);
        int giveUpCount = 0;
        char charBuf;
        std::string error;
        ssize_t readStatus = read(std_err, &charBuf, 1);

        // Try to read each sec (up to 10 before give up)
        while (readStatus == -1 && giveUpCount < 10) {
            sleep(1);
            readStatus = read(std_err, &charBuf, 1);
            giveUpCount++;
        }

        // Read until the EOF, if the stderr is available
        if (readStatus != -1) {
            error.push_back(charBuf);
            while (read(std_err, &charBuf, 1) != 0) {
                error.push_back(charBuf);
            }
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
            Glib::RefPtr<Gio::File> pidFile = Gio::File::create_for_path(this->BACKGROUND_RUN_PATH);
            Glib::RefPtr<Gio::FileOutputStream> writerStream;
            if (pidFile->query_exists()) {
                writerStream = pidFile->replace();
            }
            else {
                writerStream = pidFile->create_file();
            }
            writerStream->write(std::to_string(child_pid));
            writerStream->flush();
            writerStream->close();
            writerStream.reset();
        }

    }
    catch (Glib::SpawnError& ex) {
        std::cerr << ex.what() << std::endl;
    }
}

void MainWindow::stopBackgroundProcess() {
    Glib::RefPtr<Gio::File> pidFile = Gio::File::create_for_path(this->BACKGROUND_RUN_PATH);
    Glib::RefPtr<Gio::FileInputStream> readerStream;
    if (pidFile->query_exists()) {
        readerStream = pidFile->read();

        std::string pid;
        char charBuffer;
        while (readerStream->read(&charBuffer, 1) != 0) {
            pid.push_back(charBuffer);
        }
        std::vector<std::string> argumentVector;
        argumentVector.push_back("/usr/bin/pkexec");
        argumentVector.push_back("/usr/bin/kill");
        argumentVector.push_back("-s");
        argumentVector.push_back("SIGINT");
        argumentVector.push_back(pid);

        try {
            int std_err;
            Glib::spawn_async_with_pipes(
                    Utility::APPLICATION_PATH,
                    argumentVector,
                    Glib::SPAWN_DO_NOT_REAP_CHILD,
                    sigc::slot0<void>(),
                    nullptr,
                    nullptr,
                    nullptr,
                    &std_err
            );
        }
        catch (Glib::SpawnError& ex) {
            std::cerr << ex.what() << std::endl;
        }

        readerStream->close();
        readerStream.reset();
        //Remove pid file
        pidFile->remove();
    }
    else {
        std::cerr << "Pid file not exists" << std::endl;
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
        this->statusLabel->set_label("Hotplugger is <b>enabled</b>");
    }
    else {
        this->stopBackgroundProcess();

        // Enables the widgets
        this->setControlWidgetsState(true);
        this->statusLabel->set_label("Hotplugger is <b>disabled</b>");
    }
}