/* ---------------------------------------------------------------------------
** see LICENSE.md
**
** Sequencer.cpp
**  Sequencer main class
**
** Author: rnarmala
** -------------------------------------------------------------------------*/

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
    unlink(config->getPIDFile().c_str());
    saveToDatabase();
	exit(signal);
}

// Check old prcoess is running or not
void checkAndExitOnOldPid() {
    string oldPID;

    readFromFile(config->getPIDFile(), oldPID);
    if (!oldPID.length()) {
        return;
    }

    int pid = 0;

    try {
        std::string::size_type sz;
        pid = std::stoi(oldPID, &sz);
    } catch (const std::invalid_argument& ex) {
        std::cerr << "Invalid PID found in the PID file " << config->getPIDFile() << "\n";
        LOG(ERROR) << "Invalid PID found in the PID file " << config->getPIDFile() << "\n";
        exit(EXIT_FAILURE);
    }

    if (0 == kill(pid, 0)) {
        std::cerr << "Process already running, PID found in the PID file " << config->getPIDFile() << "\n";
        LOG(ERROR) << "Process already running, PID found in the PID file " << config->getPIDFile() << "\n";
        exit(EXIT_FAILURE);
    }
}

// dump new pid to PID file
int dumpNewPid() {
    char pid[32];

    sprintf(pid, "%ld", (long)getpid());

    //cout << "PID written to: " << pid << ", " << config->getPIDFile() << "\n";
    writeToFile(config->getPIDFile().c_str(), pid);

    return 1;
}

// daemonize this process
void daemonize() {
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

// config Logger
void configLogger() {
    el::Configurations logConf;
    logConf.setToDefault();
    logConf.setGlobally(LogConfigType::Enabled, config->logEnabled ? string("true") : string("false"));
    logConf.setGlobally(LogConfigType::Filename, config->getLogFile());
    logConf.setGlobally(LogConfigType::ToStandardOutput, string("false"));
    logConf.setGlobally(LogConfigType::MaxLogFileSize, string("2048"));
    if (config->logEnabled && !config->debug) {
        logConf.set(LogLevel::Info, LogConfigType::Enabled, "false");
        logConf.set(LogLevel::Trace, LogConfigType::Enabled, "false");
        logConf.set(LogLevel::Debug, LogConfigType::Enabled, "false");
        logConf.set(LogLevel::Verbose, LogConfigType::Enabled, "false");
    }
    el::Loggers::reconfigureLogger("default", logConf);
}

// Save data to database
void saveToDatabase() {

    string s;
    s.append(std::to_string(counters.size()));
    s.append("\n");

    for (auto it = counters.begin(); it != counters.end(); it++) {
        s.append(it->first);
        s.append(":");
        s.append(std::to_string(it->second->value));
        s.append("\n");
    }

    writeToFile(config->getDataFile().c_str(), s.c_str());
    //LOG(INFO) << "Data Written: " << s;
}

// read data from database
int readFromDatabase() {
    if (!fileExists(config->getDataFile())) {
        LOG(INFO) << "data file is empty " << config->getDataFile() << "\n";
        return 0;
    }

    counters.clear();

    string data;
    if (!readFromFile(config->getDataFile().c_str(), data)) {
        std::cerr << "Unable to read from data file " << config->getDataFile() << "\n";
        LOG(ERROR) << "Unable to read from data file " << config->getDataFile() << "\n";
        exit(EXIT_FAILURE);
    }

    uBigInt len = data.length();
    uBigInt line = 0;

    string key, val;
    bool flip = false;

    for (uBigInt i = 0; i < len; i++) {
        switch (data[i]) {
            case ':':
                flip = true;
                break;

            case '\n':
                if (line == 0) {
                    // number of elements
                    //toHugeInt num = toHugeInt(key);
                } else {
                    counters[key] = new Sequence(toHugeInt(val));
                }

                key.clear();
                val.clear();
                flip = false;
                line++;
                break;

            default:
                if (flip) {
                    val.append(1, data[i]);
                } else {
                    key.append(1, data[i]);
                }
                break;
        }
    }

    LOG(INFO) << "Data Read: " << (line-1) << " records read";
}


 // Sequencer
Sequencer::Sequencer() {
    isListening = 0;
    isSaving = 0;
}

Sequencer::Sequencer(const Sequencer&) {
}

Sequencer::~Sequencer() {
}

void Sequencer::sentinelTask() {
    LOG(INFO) << "sentinelTask started ... ";

    while (1) {
        //LOG(INFO) << "sentinelTask ... ";

        if (isListening == 1) {
            isListening = 0;
            delete listenThread;

            LOG(INFO) << "threadListener starting ... ";
            listenThread = new Thread(&Sequencer::listenerTask, Sequencer());
            listenThread->join();
        }

        if (isSaving == 1) {
            isSaving = 0;
            delete savingThread;

            LOG(INFO) << "threadSaving starting ... ";
            savingThread = new Thread(&Sequencer::saveToDiskTask, Sequencer());
            savingThread->detach();
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
}

void Sequencer::listenerTask() {
    isListening = 2;

    SeqListener *conn = new SeqListener(config->port);
    conn->listenNow();

    delete conn;
    isListening = 1;
}

void Sequencer::saveToDiskTask() {
    LOG(INFO) << "saveToDiskTask started ... ";

    isSaving = 2;

    while (1) {
        //LOG(INFO) << "saveToDiskTask ... ";

        saveToDatabase();

        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    isSaving = 1;
}

void Sequencer::start() {
    readFromDatabase();

    LOG(INFO) << "threadSentinel starting ... ";
    sentinelThread = new Thread(&Sequencer::sentinelTask, Sequencer());

    LOG(INFO) << "threadSaving starting ... ";
    savingThread = new Thread(&Sequencer::saveToDiskTask, Sequencer());

    LOG(INFO) << "threadListener starting ... ";
    listenThread = new Thread(&Sequencer::listenerTask, Sequencer());

    savingThread->detach();
    listenThread->join();
    sentinelThread->join();
}
