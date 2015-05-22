#include "Sequencer.h"
#include "SeqConfig.cpp"
#include "Sequence.cpp"
#include "SequencerTask.cpp"
#include "SeqListener.cpp"
#include "Sequencer.cpp"


// main
int main(const int argc, char *argv[])
{
    config = new SeqConfig();
    
    //0. Initialize Global Variables
    runningClients = 0;

    // 1. Read Configuration
    srand(time(NULL));
    
    // create a command line argument parser
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

    config->port = cmdLine.get<int>("port");
    config->setHomeDir(cmdLine.get<string>("home-dir"));
    config->logEnabled = !cmdLine.exist("disable-logging");
    config->debug = cmdLine.exist("debug");
    
    if (cmdLine.get<string>("config").length()) {
        config->load(cmdLine.get<string>("config"));
    }

    config->validate();

    if (cmdLine.exist("get-pid-file")) {
        std::cout << config->getPIDFile();
        exit(EXIT_SUCCESS);
    }

    // check old pID exist
    checkAndExitOnOldPid();

    // 2. Config Logging
    configLogger();
    // Config Logging - END


    if (cmdLine.exist("daemon")) {
        daemonize();
    }

    // write PID to file
    dumpNewPid();

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