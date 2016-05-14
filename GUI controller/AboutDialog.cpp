#include "AboutDialog.h"

AboutDialog::AboutDialog(BaseObjectType *cobject, const Glib::RefPtr<Gtk::Builder>& builder):
    Gtk::AboutDialog(cobject) {

    Gtk::Button* btnClose = nullptr;
    builder->get_widget("btnClose", btnClose);
    btnClose->signal_clicked().connect(sigc::mem_fun0(*this, &AboutDialog::quit));
}

AboutDialog::~AboutDialog() {

}

void AboutDialog::quit() {
    this->close();
}