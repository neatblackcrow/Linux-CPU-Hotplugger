#include <gtkmm.h>

#ifndef LINUX_CPU_HOTPLUGGER_ABOUTDIALOG_H
#define LINUX_CPU_HOTPLUGGER_ABOUTDIALOG_H

class AboutDialog : public Gtk::AboutDialog {
public:
    AboutDialog(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& builder);
    ~AboutDialog();
protected:
    void quit();
};

#endif //LINUX_CPU_HOTPLUGGER_ABOUTDIALOG_H
