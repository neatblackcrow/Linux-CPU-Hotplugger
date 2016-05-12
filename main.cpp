#include <iostream>
#include <fstream>
#include <unistd.h>
#include <signal.h>

using namespace std;

struct {
    unsigned int min_core_user;
    unsigned int max_core_user;
    unsigned int min_threshold;
    unsigned int max_threshold;

    unsigned int min_core_system = 1;
    unsigned int max_core_system;
} cpuInfo;

struct CPUTime {
    unsigned int userLoad, niceLoad, systemLoad, idleLoad;
} latestCPUTime;

unsigned int sleepTime;
const string procStatPath = "/proc/stat";
const string sysCpuPath = "/sys/devices/system/cpu/cpu?/online";
unsigned int focusingCpu;

enum ToggleState {
    OFF,
    ON
};

static inline CPUTime getCPUTime() {
    ifstream statReader(procStatPath);
    CPUTime currentCPUTime;
    string dummyVar;    // ignore the 'cpu' string
    statReader >> dummyVar >> currentCPUTime.userLoad >> currentCPUTime.niceLoad >> currentCPUTime.systemLoad >> currentCPUTime.idleLoad;
    statReader.close();
    return currentCPUTime;
}

static void initSystemCpuInfo() {
    // Get the maximum number of CPUs
    ifstream cpuPresent("/sys/devices/system/cpu/present");
    string readLine;
    getline(cpuPresent, readLine);
    cpuInfo.max_core_system = stoul(readLine.substr(readLine.find("-")+1, readLine.length()));
    cpuPresent.close();

    // Initialize the latest CPU time
    latestCPUTime = getCPUTime();
    sleep(2);    // Sleep to prevent floating point exception (divide by zero)
}

static inline float getCurrentCPUUtilization() {
    CPUTime currentCPUTime = getCPUTime();
    float cpu_use = (currentCPUTime.userLoad-latestCPUTime.userLoad)+(currentCPUTime.niceLoad-latestCPUTime.niceLoad)+(currentCPUTime.systemLoad-latestCPUTime.systemLoad);
    float cpu_total = cpu_use+(currentCPUTime.idleLoad-latestCPUTime.idleLoad);
    latestCPUTime = currentCPUTime;
    return (cpu_use/cpu_total)*100;
}

static inline void cpuCoreToggle(unsigned int coreNumber, ToggleState desiredState) {
    string sysCPU = sysCpuPath;
    sysCPU.replace(27, 1, to_string(coreNumber));
    ofstream coreActivator(sysCPU);
    coreActivator << desiredState;    // OFF=0 and ON=1 (enum type)
    coreActivator.close();
}

static inline void runLoop() {
    /*
     * Assume that the system use all of it CPUs
     *
     * First, we need to turn off the exceeded CPU(s) (based on --max-core-user)
     *
     */
    focusingCpu = cpuInfo.max_core_system;
    while (focusingCpu > (cpuInfo.max_core_user-1)) {
        cpuCoreToggle(focusingCpu, OFF);
        focusingCpu--;
    }

    while (true) {
        float loadAVG = getCurrentCPUUtilization();
        cout << loadAVG << endl;
        if ((loadAVG > cpuInfo.max_threshold) && (focusingCpu < cpuInfo.max_core_user-1)) {
            cpuCoreToggle(focusingCpu, ON);
            focusingCpu++;
            cout << "increase" << endl;
        }
        else if ((loadAVG < cpuInfo.min_threshold) && (focusingCpu > cpuInfo.min_core_user-1)) {
            cpuCoreToggle(focusingCpu, OFF);
            focusingCpu--;
            cout << "decrease" << endl;
        }
        sleep(sleepTime);
    }
}

static void signalReceived(int sigNum) {
    // Restore all the CPUs
    string sysCPU = sysCpuPath;
    unsigned long minCPU = cpuInfo.min_core_system;
    unsigned long maxCPU = cpuInfo.max_core_system;
    while (minCPU <= maxCPU) {
        sysCPU.replace(27, 1, to_string(minCPU));
        ofstream coreActivator(sysCPU);
        coreActivator << "1";
        coreActivator.close();
        minCPU++;
    }
    _exit(0);
}

static void initSignalHandlers() {
    signal(SIGTERM, signalReceived);
    signal(SIGINT, signalReceived);
    signal(SIGKILL, signalReceived);
}


int main(int argc, char* argv[]) {
    initSystemCpuInfo();
    initSignalHandlers();
    if (argc == 11) {
        for (int i=1; i<argc; i++) {
            string arg_name(argv[i]);
            i++;
            if (arg_name.compare("--min-core") == 0) {
                cpuInfo.min_core_user = stoul(string(argv[i]).c_str());
                if (cpuInfo.min_core_user < 1) {
                    cerr << "At least one CPU core must be online" << endl;
                    return 1;
                }
            }
            else if (arg_name.compare("--max-core") == 0) {
                cpuInfo.max_core_user = stoul(string(argv[i]).c_str());
                if (cpuInfo.max_core_user > cpuInfo.max_core_system+1) {
                    cerr << "Maximum online cores can't exceed a hardware limit" << endl;
                    return 1;
                }
            }
            else if (arg_name.compare("--min-threshold") == 0) {
                cpuInfo.min_threshold = stoul(string(argv[i]).c_str());
                if (cpuInfo.min_threshold > 100 || cpuInfo.min_threshold < 10) {
                    cerr << "Threshold value must be between 10 and 100" << endl;
                    return 1;
                }
            }
            else if (arg_name.compare("--max-threshold") == 0) {
                cpuInfo.max_threshold = stoul(string(argv[i]).c_str());
                if (cpuInfo.max_threshold > 100 || cpuInfo.max_threshold < 10) {
                    cerr << "Threshold value must be between 10 and 100" << endl;
                    return 1;
                }
            }
            else if (arg_name.compare("--sleep") == 0) {
                sleepTime = stoul(string(argv[i]).c_str());
                if (sleepTime < 1) {
                    cerr << "Sleep timeout must be more than 0" << endl;
                    return 1;
                }
            }
        }

        if (cpuInfo.min_core_user > cpuInfo.max_core_user) {
            cerr << "The minimum core online must be less than or equal to the maximum core online" << endl;
            return 1;
        }
        else if (cpuInfo.min_threshold > cpuInfo.max_threshold) {
            cerr << "The minimum threshold must be less than or equal to the maximum threshold" << endl;
            return 1;
        }

        runLoop();

    }
    else {
        cerr << "You must enter all the arguments required" << endl;
        return 1;
    }
    return 0;
}