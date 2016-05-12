#include <gtkmm.h>

int main(int argc, char* argv[]) {
    Glib::RefPtr<Gtk::Application> application = Gtk::Application::create(argc, argv, "org.fieldfirst.CPUHotplugger");
    Gtk::Window test;
    test.set_size_request(200, 200);
    test.show();
    return application->run(test);
}