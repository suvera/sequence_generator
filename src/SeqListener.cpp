/* ---------------------------------------------------------------------------
** see LICENSE.md
**
** Sequencer.cpp
**  Sequencer main class
**
** Author: rnarmala
** -------------------------------------------------------------------------*/
#ifndef _SEQUENCE_LISTENER
#define _SEQUENCE_LISTENER

// he backlog argument defines the maximum length to which
// the queue of pending connections for sockfd may grow.
// If a connection request arrives when the queue is full,
// the client may receive an error with an indication of ECONNREFUSED
#define LISTEN_BACKLOG 50


// SeqListener
class SeqListener {
protected:
    int port;

    string host;

    int conn;

    std::atomic<bool> listening;

    // open socket, bind and then listen
    int start() {
        conn = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);

        // non-blocking mode
        //fcntl(conn, F_SETFL, O_NONBLOCK);

        struct sockaddr_in address;
        memset(&address, 0, sizeof(address));

        address.sin_family = PF_INET;
        address.sin_port = htons(port);

        if (host.size() > 0) {
            inet_pton(PF_INET, host.c_str(), &(address.sin_addr));
        } else {
            address.sin_addr.s_addr = INADDR_ANY;
        }

        int option_value = 1;

        // SO_REUSEADDR: Specifies that the rules used in validating addresses supplied to bind()
        // should allow reuse of local addresses, if this is supported by the protocol.
        setsockopt(conn, SOL_SOCKET, SO_REUSEADDR, &option_value, sizeof(option_value));

        int success = bind(conn, (struct sockaddr*)&address, sizeof(address));

        if (success != 0) {
            LOG(ERROR) << "Socket bind() failed";
            //perror("Socket bind() failed");
            close(conn);
            return success;
        }

        success = listen(conn, LISTEN_BACKLOG);

        if (success != 0) {
            LOG(ERROR) << "Socket listen() failed";
            //perror("Socket listen() failed");
            close(conn);
            return success;
        }

        return success;
    }

public:

    SeqListener() {
    }

    SeqListener(int p) {
        port = p;
    }

    ~SeqListener() {
    }

    SeqListener(const SeqListener&) {
    }

    // listenThread
    void listenThread() {
        struct sockaddr_in clientAddress;
        socklen_t inAddrLen = sizeof(clientAddress);
        memset(&clientAddress, 0, sizeof(clientAddress));
        int descriptor = 0;

        while (descriptor = ::accept(conn, (struct sockaddr*)&clientAddress, &inAddrLen)) {
            int found = -1;
            for (auto it = seqTasks.cbegin(); it != seqTasks.cend(); ++it ) {
                if (it->ready == 0) {
                    found = it - seqTasks.begin();
                    break;
                }
            }

            if (found == -1) {
                LOG(ERROR) << "Too many connections, allowed limit exhausted!";
                sendResponse(descriptor, INVALID_TOO_MANY_CONNECTIONS_JSON);
                close(descriptor);
            } else {
                seqTasks[found].clientAddress = clientAddress;
                seqTasks[found].descriptor = descriptor;
            }
        }
    }

    // listenNow
    int listenNow() {
        if (listening) {
            return 0;
        }

        start();

        listening = true;

        Thread lt (&SeqListener::listenThread, this);

        for (int i = 0; i < config.threads; ++i) {
            seqThreads.push_back(Thread(runSequencerTask, i));
            seqTasks.push_back(SequencerTask(i));
        }

        lt.join();
        for (int i = 0; i < config.threads; ++i) {
            seqThreads[i].join();
        }
    }

};



#endif