#include <cstdlib>
#include <iostream>
#include <chrono>
#include <thread>
#include <mutex>
#include <ctime>
#include <unordered_map>
#include <ctime>
#include <algorithm> 
#include <functional>
#include <locale>
#include <cctype>
#include <cmdline.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <glob.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <time.h>
#include <atomic>
#include <sys/wait.h>
#include <signal.h>
#include <vector>
#include <uuid/uuid.h>
#define ELPP_DEFAULT_LOG_FILE "/tmp/sequencer-init.log"
#include "easylogging++.h"

typedef el::Level LogLevel;
typedef el::ConfigurationType LogConfigType;

INITIALIZE_EASYLOGGINGPP

using std::cout;
using std::endl;
using std::string;
using std::unordered_map;
using std::vector;
using std::atomic;
using namespace std::chrono;
using std::chrono::system_clock;
using namespace std::this_thread;

typedef unsigned long long uHugeInt;
typedef unsigned long uBigInt;
typedef unordered_map<string, string> StringMap;
typedef std::thread Thread;
extern std::atomic<int> runningClients;
std::atomic<int> runningClients;

#include "Sequence.h"
extern unordered_map<string, Sequence*> counters;
unordered_map<string, Sequence*> counters;

#include "utils.h"
#include "SeqConfig.cpp"
extern SeqConfig config;
SeqConfig config;
#include "Sequence.cpp"
#include "SeqCommand.cpp"
#include "SeqListener.cpp"
#include "Sequencer.cpp"

// do something on death
void signalHandler(int signal) {
    switch (signal) {
        case SIGCHLD:
            std::cerr << "Cought signal SIGCHLD\n";
            break;
        case SIGHUP:
            std::cerr << "Cought signal SIGHUP\n";
            break;
        case SIGABRT:
            std::cerr << "Cought signal SIGABRT\n";
            break;
        case SIGINT:
            std::cerr << "Cought signal SIGINT\n";
            break;
        case SIGTERM:
            std::cerr << "Cought signal SIGTERM\n";
            break;
        default:
            std::cerr << "Cought signal " << signal << "\n";
            break;
    }
    
	wait(NULL);
    unlink(config.getPIDFile().c_str());
    saveToDatabase();
	exit(signal);
}

// main
int main(const int argc, char *argv[])
{
    //0. Initialize Global Variables
    runningClients = 0;

    // 1. Read Configuration
    srand(time(NULL));
    
    // create a parser
    cmdline::parser cmdLine;
    cmdLine.set_program_name("sequencer");

    cmdLine.add<int>("port", 'p', "port number", false, SEQ_DEFAULT_PORT, cmdline::range(1, 65535));
    cmdLine.add<string>("home-dir", 'd', "Seq Home dir path", false, SEQ_DEFAULT_HOME);
    cmdLine.add<string>("config", 'c', "Config INI file path", false, "");
    cmdLine.add("daemon", 'D', "daemon mode");
    cmdLine.add("disable-logging", 'x', "Disable logging");
    cmdLine.add("get-pid-file", 'I', "Get PID file location");
    cmdLine.add("debug", 'g', "Debug Logging");
    cmdLine.add("help", 'h', "Display Help");
    cmdLine.add("verbose", 'v', "Be verbose");

    cmdLine.parse_check(argc, argv);

    if (cmdLine.exist("help")) {
        std::cerr << cmdLine.usage();
        exit(EXIT_SUCCESS);
    }

    config.port = cmdLine.get<int>("port");
    config.setHomeDir(cmdLine.get<string>("home-dir"));
    config.logEnabled = !cmdLine.exist("disable-logging");
    config.debug = cmdLine.exist("debug");
    
    if (cmdLine.get<string>("config").length()) {
        config.load(cmdLine.get<string>("config"));
    }

    config.validate();

    if (cmdLine.exist("get-pid-file")) {
        std::cout << config.getPIDFile();
        exit(EXIT_SUCCESS);
    }
    

    // 2. Config Logging
    el::Configurations logConf;
    logConf.setToDefault();
    logConf.setGlobally(LogConfigType::Enabled, config.logEnabled ? string("true") : string("false"));
    logConf.setGlobally(LogConfigType::Filename, config.getLogFile());
    logConf.setGlobally(LogConfigType::ToStandardOutput, string("false"));
    logConf.setGlobally(LogConfigType::MaxLogFileSize, string("2048"));
    if (config.logEnabled && !config.debug) {
        logConf.set(LogLevel::Info, LogConfigType::Enabled, "false");
        logConf.set(LogLevel::Trace, LogConfigType::Enabled, "false");
        logConf.set(LogLevel::Debug, LogConfigType::Enabled, "false");
        logConf.set(LogLevel::Verbose, LogConfigType::Enabled, "false");
    }
    el::Loggers::reconfigureLogger("default", logConf);
    // Config Logging - END


    if (cmdLine.exist("daemon")) {
        pid_t pid = fork();

        if (pid == 0) {
            // child process
        } else if (pid > 0) {
            // parent process
            cout << "Success!\n";
            exit(EXIT_SUCCESS);
        } else {
            // fork failed
            std::cerr << "fork() failed! Could not daemon-ize this process\n";
            exit(EXIT_FAILURE);
        }
    }

    // write PID to file
    char pid[32];
    sprintf(pid, "%ld", (long)getpid());
    //cout << "PID written to: " << pid << ", " << config.getPIDFile() << "\n";
    writeToFile(config.getPIDFile().c_str(), pid);

    // handle signals
    signal(SIGCHLD, signalHandler);
    signal(SIGHUP, signalHandler);
    //signal(SIGABRT, signalHandler);
    signal(SIGINT, signalHandler);
    //signal(SIGTERM, signalHandler);

    Sequencer *app = new Sequencer();
    app->start();

    return 0;
}