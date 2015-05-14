#include <cstdlib>
#include <iostream>
#include <chrono>
#include <thread>
#include <mutex>
#include <ctime>
#include <unordered_map>
#include <ctime>
#include <cmdline.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <time.h>
#include <atomic>
#include <sys/wait.h>
#include <signal.h>
#include <vector>
#include "easylogging++.h"

typedef el::Level LogLevel;
typedef el::ConfigurationType LogConfigType;

INITIALIZE_EASYLOGGINGPP

void signalHandler(int signal) {
    printf("Cought signal %d!\n",signal);
	while(wait(NULL) > 0);
}


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
typedef unordered_map<string, string> StringMap;
typedef std::thread Thread;

extern unordered_map<string, atomic<uHugeInt>> counters;
unordered_map<string, atomic<uHugeInt>> counters;

#include <utils.h>
#include <SeqListener.cpp>

#include <SeqConfig.cpp>
extern SeqConfig config;
SeqConfig config;

#include <Sequencer.cpp>

int main(const int argc, char *argv[])
{
    // 1. Read Config
    srand(time(NULL));

    // create a parser
    cmdline::parser cmdLine;
    cmdLine.set_program_name("sequencer");

    cmdLine.add<int>("port", 'p', "port number", false, SEQ_DEFAULT_PORT, cmdline::range(1, 65535));
    cmdLine.add<string>("data-dir", 'd', "data dir path", false, SEQ_DEFAULT_DATA_DIR);
    cmdLine.add<string>("log-file", 'l', "Log file path", false, SEQ_DEFAULT_LOG_FILE);
    cmdLine.add<string>("pid-file", 'i', "pid file path", false, SEQ_DEFAULT_PID_FILE);
    cmdLine.add("daemon", 'D', "daemon mode");
    cmdLine.add("help", 'h', "Display Help");
    cmdLine.add("verbose", 'v', "Be verbose");

    cmdLine.parse_check(argc, argv);

    if (cmdLine.exist("help")) {
        std::cerr << cmdLine.usage();
        exit(EXIT_SUCCESS);
    }

    config.port = cmdLine.get<int>("port");
    config.dataDir = cmdLine.get<string>("data-dir");
    config.logFile = cmdLine.get<string>("log-file");
    config.pidFile = cmdLine.get<string>("pid-file");

    config.validate();

    // 2. Config Logging
    el::Configurations logConf;
    logConf.setToDefault();
    logConf.setGlobally(LogConfigType::Enabled, config.logEnabled ? string("true") : string("false"));
    logConf.setGlobally(LogConfigType::Filename, config.logFile);
    logConf.setGlobally(LogConfigType::ToStandardOutput, string("false"));
    logConf.setGlobally(LogConfigType::MaxLogFileSize, string("2048"));
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
            std::cerr << "fork() failed! Could not daemonize this process\n";
            exit(EXIT_FAILURE);
        }

        // handle signals
        signal(SIGCHLD, signalHandler);
        signal(SIGHUP, signalHandler);
    }

    Sequencer *app = new Sequencer();
    app->start();

    return 0;
}