#include <syslog.h>
#include <iostream>
#include <fstream>

using namespace std;

int main(int argc, char* argv[]) {
    ifstream loadAVG("/proc/loadavg");
    if (loadAVG.is_open()) {
        std::string line;
        getline(loadAVG, line);
        cout << line << endl;
    }
    else {
    }
    return 0;
}