/* ---------------------------------------------------------------------------
** see LICENSE.md
**
** SeqConfig.cpp
**  Configuration goes here
**
** Author: rnarmala
** -------------------------------------------------------------------------*/

// Config
void SeqConfig::setHomeDir(string dir) {
    seqHome = dir;
}

// return log file
string SeqConfig::getLogFile() {
    string file = seqHome;
    return file.append("/").append("sequencer.log");
}

// return pid file
string SeqConfig::getPIDFile() {
    string file = seqHome;
    return file.append("/").append("sequencer.pid");
}

// return data dir
string SeqConfig::getDataDir() {
    string file = seqHome;
    return file.append("/").append("data");
}

// return data file
string SeqConfig::getDataFile() {
    string file = getDataDir();
    return file.append("/").append("sequencer.db");
}



// load configuration from ini file
void SeqConfig::load(const string& filePath) {
    StringMap mp;
    if (parseIniFile(filePath.c_str(), mp) < 0) {
        std::cerr << "Error: Could not read configuration from INI file " << filePath << ".\n";
        exit(EXIT_FAILURE);
    }

    if (mp.count("port")) {
        port = toInt(mp["port"]);
    }

    if (mp.count("logging.enabled")) {
        logEnabled = toBool(mp["logging.enabled"]);
    }

    if (mp.count("debug")) {
        debug = toBool(mp["debug"]);
    }

    if (mp.count("home")) {
        seqHome = mp["home"];
    }

    if (mp.count("max_clients")) {
        maxConnections = toInt(mp["max_clients"]);
    }
}



// validate
int SeqConfig::validate() {

    if (port < 1 || port > 65535) {
        std::cerr << "Error: Invalid port " << port << ".\n";
        exit(EXIT_FAILURE);
    }

    if (maxConnections <= 0) {
        std::cerr << "Error: Maximum Clients configuration must be positive number " << maxConnections << ".\n";
        exit(EXIT_FAILURE);
    }

    struct stat buffer;
    int status = stat(seqHome.c_str(), &buffer);

    if (status == -1) {
        std::cerr << "Error: Something wrong with Sequence Home folder " << seqHome << ", please check if it exists and or writable by this process.\n";
        exit(EXIT_FAILURE);
    }

    if (!S_ISDIR(buffer.st_mode)) {
        std::cerr << "Error: Sequence Home is not a directory " << seqHome << ".\n";
        exit(EXIT_FAILURE);
    }

    status = stat(getDataDir().c_str(), &buffer);

    if (status == -1) {
        std::cerr << "Error: Something wrong with Data Directory " << getDataDir() << ", please check if it exists and or writable by this process.\n";
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

