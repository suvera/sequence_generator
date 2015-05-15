class Sequencer {
public:
    Thread *listenThread;

    Thread *logThread;

    Thread *sentinelThread;

    std::atomic<int> isListening;

    // Methods
    Sequencer() {
        isListening = 0;
    }

    Sequencer(const Sequencer&) {
    }

    ~Sequencer() {
    }

    void sentinelTask() {
        if (isListening == 1) {
            delete listenThread;
            listenThread->join();
        }
    }

    void listenerTask() {
        isListening = 2;

        SeqListener *conn = new SeqListener(config.port);
        conn->listenNow();

        delete conn;
        isListening = 1;
    }

    void threadListener() {
        listenThread = new Thread(&Sequencer::listenerTask, Sequencer());
        listenThread->join();
    }

    void threadSentinel() {
        sentinelThread = new Thread(&Sequencer::sentinelTask, Sequencer());
        sentinelThread->join();
    }

    void start() {
        this->threadSentinel();
        this->threadListener();
    }
};