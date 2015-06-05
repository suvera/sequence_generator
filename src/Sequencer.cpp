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
    saveToDatabase(true);
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
    
    if (config->logEnabled) {
        //cout << "Logging enabled " << config->getLogFile() << "\n";
    }
    
    if (config->debug) {
        //cout << "Debugging mode enabled " << config->getLogFile() << "\n";
    }
    
    if (config->logEnabled && !config->debug) {
        logConf.set(LogLevel::Info, LogConfigType::Enabled, "false");
        logConf.set(LogLevel::Trace, LogConfigType::Enabled, "false");
        logConf.set(LogLevel::Debug, LogConfigType::Enabled, "false");
        logConf.set(LogLevel::Verbose, LogConfigType::Enabled, "false");
    }
    el::Loggers::reconfigureLogger("default", logConf);
}

void _syncDbStream() {
    dbStream.flush();
    
    if (dbStream.fail()) {
        LOG(ERROR) << "Failed, The Disk synchronization operation failed due to i/0 error  " << config->getDataFile();
    }
    
    if (dbStream.bad()) {
        LOG(ERROR) << "Failed, The Disk synchronization operation failed due to Either an insertion on the stream failed, or some other error happened " << config->getDataFile();
    }
}

void _writeDBSteram(const char* data, const uBigInt offset, const uBigInt len) {
    dbStream.seekp(offset, ios::beg);

    if (dbStream.fail()) {
        LOG(ERROR) << "Failed, seekp to offset " << offset << " failed due to i/0 error " << config->getDataFile();
    }

    if (dbStream.bad()) {
        LOG(ERROR) << "Failed, seekp to offset " << offset << " failed due to Either an insertion on the stream failed, or some other error happened " << config->getDataFile();
    }
    
    for (int i=0; i < len; i++) {
        dbStream.put(*(data+i));
    }
    
    // due to null terminated characters in the middle, write does not work here
    //dbStream.write(data, len);

    if (dbStream.fail()) {
        LOG(ERROR) << "Failed, writing to database failed due to i/0 error " << config->getDataFile();
    }

    if (dbStream.bad()) {
        LOG(ERROR) << "Failed, writing to database failed due to Either an insertion on the stream failed, or some other error happened " << config->getDataFile();
    }
}

// Save data to database
void saveToDatabase(bool force) {
    static uBigInt LAST_END_POS = MAX_SEQ_LENGTH;
    
    if (!dbStream.is_open()) {
        dbStream.open(config->getDataFile().c_str(), fstream::in | fstream::out | fstream::binary);
    }
    
    if (!dbStream.is_open() || !dbStream.good()) {
        LOG(ERROR) << "Unable to open database file for write. Good: " << dbStream.good() << ", Fail: " << dbStream.fail() << ", Bad: " << dbStream.bad() << ", Eof: " << dbStream.eof() << ", " << config->getDataFile().c_str();
        dbStream.close();
        return;
    }
    
    bool totalUpdated = false, dirty = false;
    
    for (auto it = counters.cbegin(); it != counters.cend(); it++) {
        if (force || it->second->dirty) {
            it->second->dirty = false;
            dirty = true;
            
            // update total
            if (!totalUpdated) {
                totalUpdated = true;
                
                string ts = std::to_string(counters.size());
                char t[MAX_SEQ_LENGTH];
                
                int i = 0;
                for (auto c = ts.cbegin(); c != ts.cend(); ++c) {
                    t[i++] = *c;
                }
                
                _writeDBSteram(t, 0, MAX_SEQ_LENGTH);
            }
            
            if (!it->second->offset) {
                // add at the end of the file
                it->second->offset = LAST_END_POS;
                
                // This is okay
                LAST_END_POS += MAX_KEY_LENGTH + MAX_SEQ_LENGTH;
            }
            
            string val = std::to_string(it->second->value);
            string key = it->first;
            
            char s[MAX_KEY_LENGTH + MAX_SEQ_LENGTH];
            memset(s, 0, sizeof(s));
            
            int i = 0;
            for (auto c = key.cbegin(); c != key.cend(); ++c) {
                s[i++] = *c;
            }
            
            i = MAX_KEY_LENGTH;
            for (auto c = val.cbegin(); c != val.cend(); ++c) {
                s[i++] = *c;
            }
            
            //cout << "written bytes " << ", Key: " << it->first << ", value: " << val << "\n";
            
            _writeDBSteram(s, it->second->offset, sizeof(s));
        }
    }
    
    
    if (dirty) {
        _syncDbStream();
    }
    
}

// read data from database
int readFromDatabase() {

    if (!fileExists(config->getDataFile())) {
        LOG(INFO) << "data file does not exist! " << config->getDataFile() << "\n";
        //return 0;
        writeToFile(config->getDataFile().c_str(), "");
        //truncate(config->getDataFile().c_str(), 1024 * 1024);
    }

    if (!dbStream.is_open()) {
        dbStream.open(config->getDataFile().c_str(), fstream::in | fstream::binary);
    }
    
    if (!dbStream.is_open() || !dbStream.good()) {
        LOG(ERROR) << "Unable to open database file for read. Good: " << dbStream.good() << ", Fail: " << dbStream.fail() << ", Bad: " << dbStream.bad() << ", Eof: " << dbStream.eof() << ", " << config->getDataFile().c_str();
        dbStream.close();
        return 0;
    }
    
    counters.clear();
    
    char seq[MAX_SEQ_LENGTH], key[MAX_KEY_LENGTH];
    
    // read total number
    dbStream.seekg(0, fstream::beg);
    dbStream.read(seq, MAX_SEQ_LENGTH);
    
    if (dbStream.fail()) {
        LOG(ERROR) << "Logical error on i/o operation, while reading total count from file " << config->getDataFile();
    }
    
    uBigInt offset = MAX_SEQ_LENGTH;
    while (!dbStream.eof()) {
    
        //dbStream.seekg(offset, fstream::beg);
        dbStream.read(key, MAX_KEY_LENGTH);
        
        if (dbStream.fail()) {
            LOG(ERROR) << "Logical error on i/o operation, while reading data sequence KEY from file, offset: " << offset << ", " << config->getDataFile();
        }
        
        offset += MAX_KEY_LENGTH;
        
        //dbStream.seekg(offset, fstream::beg);
        dbStream.read(seq, MAX_SEQ_LENGTH);
        
        offset += MAX_SEQ_LENGTH;
        
        if (dbStream.fail()) {
            LOG(ERROR) << "Logical error on i/o operation, while reading data sequence VALUE from file, offset: " << offset << ", " << config->getDataFile();
        }
        
        if (strlen(key)) {
            counters[key] = new Sequence(toHugeInt(seq));
            counters[key]->dirty = false;
            counters[key]->offset = offset;
        } else {
            LOG(ERROR) << "empty KEY and VALUE found on read " << config->getDataFile();
        }
    }
    
    dbStream.close();
    return 1;
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

        saveToDatabase(false);

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
