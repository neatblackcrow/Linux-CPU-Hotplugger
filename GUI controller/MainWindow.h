#include <gtkmm.h>
#include <iostream>

#ifndef LINUX_CPU_HOTPLUGGER_MAINWINDOW_H
#define LINUX_CPU_HOTPLUGGER_MAINWINDOW_H

class MainWindow : public Gtk::ApplicationWindow {
public:
    MainWindow(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& builder);
    ~MainWindow();
protected:
    void showAbout();
    void quit();
    void toggleStatus();
private:
    const std::string BACKGROUND_RUN_PATH = "/var/run/linux_cpu_hotplugger.pid";
    Glib::RefPtr<Gtk::Builder> builder;
    Gtk::Label* statusLabel;
    Gtk::Switch* statusToggle;
    Gtk::SpinButton* minThreshold;
    Gtk::SpinButton* maxThreshold;
    Gtk::SpinButton* minCores;
    Gtk::SpinButton* maxCores;
    Gtk::SpinButton* intervals;
};

#endif //LINUX_CPU_HOTPLUGGER_MAINWINDOW_H
