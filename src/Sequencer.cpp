/* ---------------------------------------------------------------------------
** see LICENSE.md
**
** Sequencer.cpp
**  Sequencer main class
**
** Author: rnarmala
** -------------------------------------------------------------------------*/
#ifndef _SEQUENCE_SEQUENCER
#define _SEQUENCE_SEQUENCER




void saveToDatabase() {

    string s;
    s.append(std::to_string(counters.size()));
    s.append("\n");

    for (auto it = counters.begin(); it != counters.end(); it++) {
        s.append(it->first);
        s.append(":");
        s.append(std::to_string(it->second->getVal()));
        s.append("\n");
    }

    writeToFile(config.getDataFile().c_str(), s.c_str());
    //LOG(INFO) << "Data Written: " << s;
}

int readFromDatabase() {
    if (!fileExists(config.getDataFile())) {
        LOG(INFO) << "data file is empty " << config.getDataFile() << "\n";
        return 0;
    }

    counters.clear();

    string data;
    if (!readFromFile(config.getDataFile().c_str(), data)) {
        std::cerr << "Unable to read from data file " << config.getDataFile() << "\n";
        LOG(ERROR) << "Unable to read from data file " << config.getDataFile() << "\n";
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
class Sequencer {
protected:
    Thread *listenThread;

    Thread *logThread;

    Thread *sentinelThread;

    Thread *savingThread;

    std::atomic<int> isListening;

    std::atomic<int> isSaving;
    
public:

    // Methods
    Sequencer() {
        isListening = 0;
        isSaving = 0;
    }

    Sequencer(const Sequencer&) {
    }

    ~Sequencer() {
    }

    void sentinelTask() {
        LOG(INFO) << "sentinelTask started ... ";

        while (1) {
            //LOG(INFO) << "sentinelTask ... ";

            if (isListening == 1) {
                isListening = 0;
                delete listenThread;
                threadListener();
            }

            if (isSaving == 1) {
                isSaving = 0;
                delete savingThread;
                threadSaving();
            }

            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
    }

    void listenerTask() {
        isListening = 2;

        SeqListener *conn = new SeqListener(config.port);
        conn->listenNow();

        delete conn;
        isListening = 1;
    }

    void saveToDiskTask() {
        LOG(INFO) << "saveToDiskTask started ... ";

        isSaving = 2;

        while (1) {
            //LOG(INFO) << "saveToDiskTask ... ";

            saveToDatabase();

            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }

        isSaving = 1;
    }

    void threadSaving() {
        LOG(INFO) << "threadSaving starting ... ";
        savingThread = new Thread(&Sequencer::saveToDiskTask, Sequencer());
        //savingThread->join();
    }

    void threadListener() {
        LOG(INFO) << "threadListener starting ... ";
        listenThread = new Thread(&Sequencer::listenerTask, Sequencer());
        //listenThread->join();
    }

    void threadSentinel() {
        LOG(INFO) << "threadSentinel starting ... ";
        sentinelThread = new Thread(&Sequencer::sentinelTask, Sequencer());
        //sentinelThread->join();
    }

    void start() {
        readFromDatabase();

        this->threadSentinel();

        this->threadSaving();

        // somehow this should be started at last
        this->threadListener();

        savingThread->join();
        listenThread->join();
        sentinelThread->join();
    }
};

#endif