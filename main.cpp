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
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <time.h>

#include <sys/wait.h>
#include <signal.h>

void signalHandler(int signal) {
    printf("Cought signal %d!\n",signal);
	while(wait(NULL) > 0);
}


using std::cout;
using std::endl;
using std::string;
using std::unordered_map;
using namespace std::chrono;
using std::chrono::system_clock;
using namespace std::this_thread;

#include <Sequence.cpp>
extern unordered_map<string, Sequence> counters;
unordered_map<string, Sequence> counters;


#include <SeqListener.cpp>

int main(const int argc, char *argv[])
{
    srand(time(NULL));

    // create a parser
    cmdline::parser cmdLine;
    cmdLine.set_program_name("sequencer");

    cmdLine.add<int>("port", 'p', "port number", false, 5088, cmdline::range(1, 65535));
    cmdLine.add<string>("config", 'c', "Conf file path", false, "");
    cmdLine.add("daemon", 'd', "daemon mode");
    cmdLine.add("help", 'h', "Display Help");
    cmdLine.add("verbose", 'v', "Be verbose");

    cmdLine.parse_check(argc, argv);

    if (cmdLine.exist("help")) {
        std::cerr << cmdLine.usage();
        exit(0);
    }

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

    SeqListener *conn = new SeqListener(cmdLine.get<int>("port"));

    conn->listenNow();

    return 0;
}