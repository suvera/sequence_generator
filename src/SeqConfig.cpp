#define SEQ_DEFAULT_PORT 5088

#define SEQ_DEFAULT_PID_FILE "/var/run/sequencer.pid"

#define SEQ_DEFAULT_LOG_FILE "/var/log/sequencer.log"

#define SEQ_DEFAULT_DATA_DIR "/var/lib/sequencer"

// Config
struct SeqConfig {

    int port = SEQ_DEFAULT_PORT;

    string host;

    string pidFile = SEQ_DEFAULT_PID_FILE;

    bool logEnabled = true;

    LogLevel logLevel = LogLevel::Info;

    string logFile = SEQ_DEFAULT_LOG_FILE;

    string dataDir = SEQ_DEFAULT_DATA_DIR;

    // methods
    int validate() {
        struct stat buffer;

        int status = stat(dataDir.c_str(), &buffer);

        if (status == -1) {
            std::cerr << "Error: Something wrong with DataDir, please check if it exists and or writable by this process.\n";
            exit(EXIT_FAILURE);
        }

        if (!S_ISDIR(buffer.st_mode)) {
            std::cerr << "Error: DataDir is not a directory.\n";
            exit(EXIT_FAILURE);
        }

        FILE *fp = fopen(pidFile.c_str(), "w");
        if (fp == NULL) {
            std::cerr << "Error: Something wrong with PidFile, please check if it is writable by this process.\n";
            exit(EXIT_FAILURE);
        }
        fclose(fp);

        if (logEnabled) {
            FILE *fp = fopen(logFile.c_str(), "w");
            if (fp == NULL) {
                std::cerr << "Error: Something wrong with LogFile, please check if it is writable by this process.\n";
                exit(EXIT_FAILURE);
            }
            fclose(fp);
        }
    }
};
