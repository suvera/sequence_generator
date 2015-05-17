/* ---------------------------------------------------------------------------
** see LICENSE.md
**
** SeqConfig.cpp
**  Configuration goes here
**
** Author: rnarmala
** -------------------------------------------------------------------------*/
#ifndef _SEQUENCE_CONFIG
#define _SEQUENCE_CONFIG

#define SEQ_DEFAULT_PORT 5088

#define SEQ_DEFAULT_MAX_CONNECTIONS 1000

#define SEQ_DEFAULT_HOME "/var/lib/sequencer"

// Config
struct SeqConfig {

    int port = SEQ_DEFAULT_PORT;

    string host;

    bool logEnabled = true;

    bool debug = false;

    LogLevel logLevel = LogLevel::Info;

    string seqHome = SEQ_DEFAULT_HOME;

    int maxConnections = SEQ_DEFAULT_MAX_CONNECTIONS;

    // methods
    void setHomeDir(string dir) {
        seqHome = dir;
    }

    // return log file
    string getLogFile() {
        string file = seqHome;
        return file.append("/").append("sequencer.log");
    }

    // return pid file
    string getPIDFile() {
        string file = seqHome;
        return file.append("/").append("sequencer.pid");
    }

    // return data dir
    string getDataDir() {
        string file = seqHome;
        return file.append("/").append("data");
    }

    // return data file
    string getDataFile() {
        string file = getDataDir();
        return file.append("/").append("sequencer.db");
    }

    // validate
    int validate() {
        struct stat buffer;

        int status = stat(seqHome.c_str(), &buffer);

        if (status == -1) {
            std::cerr << "Error: Something wrong with Sequence Home folder, please check if it exists and or writable by this process.\n";
            exit(EXIT_FAILURE);
        }

        if (!S_ISDIR(buffer.st_mode)) {
            std::cerr << "Error: Sequence Home is not a directory.\n";
            exit(EXIT_FAILURE);
        }

        status = stat(getDataDir().c_str(), &buffer);

        if (status == -1) {
            std::cerr << "Error: Something wrong with Data Directory, please check if it exists and or writable by this process.\n";
            exit(EXIT_FAILURE);
        }

        if (!S_ISDIR(buffer.st_mode)) {
            std::cerr << "Error: DataDir is not a directory.\n";
            exit(EXIT_FAILURE);
        }

        FILE *fp = fopen(getPIDFile().c_str(), "wb");
        if (fp == NULL) {
            std::cerr << "Error: Something wrong with PidFile, please check if it is writable by this process.\n";
            exit(EXIT_FAILURE);
        }
        fclose(fp);

        if (logEnabled) {
            FILE *fp = fopen(getLogFile().c_str(), "wb");
            if (fp == NULL) {
                std::cerr << "Error: Something wrong with LogFile, please check if it is writable by this process.\n";
                exit(EXIT_FAILURE);
            }
            fclose(fp);
        }
    }

};


#endif