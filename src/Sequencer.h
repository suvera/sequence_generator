/* ---------------------------------------------------------------------------
** see LICENSE.md
**
** Sequencer.h
**    Sequencer header file
**
** Author: rnarmala
** -------------------------------------------------------------------------*/
#ifndef _SEQUENCER_H
#define _SEQUENCER_H

#include <cstdlib>
#include <climits>
#include <iostream>
#include <fstream>
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
#define ELPP_DEBUG_ERRORS
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
using std::ios;
using std::fstream;
using namespace std::chrono;
using std::chrono::system_clock;
using namespace std::this_thread;


typedef unsigned long long uHugeInt;
typedef unsigned long uBigInt;

typedef unordered_map<string, string> StringMap;
typedef std::thread Thread;

extern std::atomic<int> runningClients;
std::atomic<int> runningClients;


#include "utils.h"

// Note: update this in INVALID_CMD_KEY_JSON as well
#define MAX_KEY_LENGTH 64
// 128 bit integers
#define MAX_SEQ_LENGTH 40

#define INVALID_CMD_JSON "{\"success\": 0, \"code\": 100, \"error\": \"Invalid request.\"}\n"

#define UNKNOWN_CMD_JSON "{\"success\": 0, \"code\": 101, \"error\": \"Unknown command.\"}\n"

#define INVALID_CMD_KEY_EMPTY_JSON "{\"success\": 0, \"code\": 102, \"error\": \"Please provide a key.\"}\n"

#define INVALID_CMD_KEY_JSON "{\"success\": 0, \"code\": 103, \"error\": \"Invalid Key, Key must be alpha numeric and should start with a letter And length must be atleast 1 and must not exceed 64.\"}\n"

#define INVALID_CMD_NO_KEY_JSON "{\"success\": 0, \"code\": 104, \"error\": \"key does not exist.\"}\n"

#define INVALID_CMD_ALREADY_KEY_JSON "{\"success\": 0, \"code\": 105, \"error\": \"key already exist.\"}\n"

#define INVALID_CMD_KEY_VAL_INVALID_JSON "{\"success\": 0, \"code\": 106, \"error\": \"Inavalid value, value cannot be empty and must be a positive integer.\"}\n"

#define INVALID_CMD_UNAUTH_JSON "{\"success\": 0, \"code\": 107, \"error\": \"Authentication failed.\"}\n"

#define INVALID_TOO_MANY_CONNECTIONS_JSON "{\"success\": 0, \"code\": 108, \"error\": \"Too many connections.\"}\n"

#define INTEGER_OVERFLOW_JSON "{\"success\": 0, \"code\": 109, \"error\": \"Integer overflow.\"}\n"

#define SUCCESS_JSON_BEGIN "{\"success\": 1, \"data\": "

#define SUCCESS_JSON_END "}\n"

#define _OP_ID "op"
#define _KEY_ID "key"
#define _KEY_VAL "value"

#define _STATUS_TEXT "status"
#define _NEXT_TEXT "next"
#define _GET_TEXT "get"
#define _CREATE_TEXT "create"
#define _SET_TEXT "set"
#define _RESET_TEXT "reset"
#define _UUID_TEXT "uuid"

enum _CMD {
    STATUS,
    NEXT,
    GET,
    CREATE,
    SET,
    UUID,
    UNKNOWN
};

#define READ_BYTES 256

#define SEQ_DEFAULT_PORT 5088

#define SEQ_DEFAULT_MAX_CONNECTIONS 1000

#define SEQ_DEFAULT_HOME "/var/lib/sequencer"


// he backlog argument defines the maximum length to which
// the queue of pending connections for sockfd may grow.
// If a connection request arrives when the queue is full,
// the client may receive an error with an indication of ECONNREFUSED
#define LISTEN_BACKLOG 50

#define SEQUENCE_START_FROM 0

typedef std::mutex Mutex;
typedef std::timed_mutex TtlMutex;

extern Mutex newSeqMtx;
Mutex newSeqMtx;

extern Mutex UUIDMtx;
Mutex UUIDMtx;

typedef std::atomic<uHugeInt> uHugeIntAtomic;
typedef std::atomic<bool> BoolAtomic;
typedef std::atomic<uBigInt> BigAtomic;


// Check old prcoess is running or not
void checkAndExitOnOldPid();

// dump new pid to PID file
int dumpNewPid();

// Signal handler
void signalHandler(int signal);

// daemon-ize
void daemonize();

// config Logger
void configLogger();

// send response to client
int sendResponse(int& descriptor, const char* resp);

void closeClientConnection(int& descriptor);

// running on a Thread
void processTask(const char* cmdStr, int cmdStrLen, int descriptor);

// Add new Sequence
void addNewSequence(const string &key, uHugeInt val);

// get new UUID
void getUUID(char *uuid);


// Sequencer Task
class SequencerTask {
public:
    SequencerTask();

    SequencerTask(const SequencerTask&);

    // run this task
    void run(int descriptor, struct sockaddr_in clientAddress);
};


// Sequencer Config
class SeqConfig {
public:
    int port = SEQ_DEFAULT_PORT;

    string host;

    bool logEnabled = true;

    bool debug = false;

    LogLevel logLevel = LogLevel::Info;

    string seqHome = SEQ_DEFAULT_HOME;

    int maxConnections = SEQ_DEFAULT_MAX_CONNECTIONS;

    // methods
    void setHomeDir(string dir);

    // return log file
    string getLogFile();

    // return pid file
    string getPIDFile();

    // return data dir
    string getDataDir();

    // return data file
    string getDataFile();

    // load configuration from ini file
    void load(const string& filePath);

    // validate
    int validate();

};


extern SeqConfig *config;
SeqConfig *config;

// SeqListener
class SeqListener {
protected:
    int port;

    string host;

    int conn;

    std::atomic<bool> listening;

    // open socket, bind and then listen
    int start();

public:

    SeqListener();

    SeqListener(int p);

    ~SeqListener();

    SeqListener(const SeqListener&);

    // listenThread
    void listenThread();

    // listenNow
    int listenNow();
};

// Sequence
class Sequence {
protected:

    string id;

    // Methods
    uHugeIntAtomic _nextVal();

public:
    uHugeIntAtomic value;
    
    BoolAtomic dirty;
    
    uBigInt offset;

    // Constructors
    Sequence();

    Sequence(uHugeInt val);
    Sequence(const Sequence&);

    ~Sequence();

    // Methods
    string getId() const;
};


extern unordered_map<string, Sequence*> counters;
unordered_map<string, Sequence*> counters;

extern fstream dbStream;
fstream dbStream;


extern uBigInt dbStreamPos;
uBigInt dbStreamPos = 0;


// Save data to database
void saveToDatabase(bool);

// read data from database
int readFromDatabase();


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
    Sequencer();

    Sequencer(const Sequencer&);

    ~Sequencer();

    void sentinelTask();

    void listenerTask();

    void saveToDiskTask();

    void start();
};



#endif