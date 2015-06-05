/* ---------------------------------------------------------------------------
** see LICENSE.md
**
** Sequencer.cpp
**  Sequencer main class
**
** Author: rnarmala
** -------------------------------------------------------------------------*/

// open socket, bind and then listen
int SeqListener::start() {
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

SeqListener::SeqListener() {
}

SeqListener::SeqListener(int p) {
    port = p;
}

SeqListener::~SeqListener() {
}

SeqListener::SeqListener(const SeqListener&) {
}

// listenThread
void SeqListener::listenThread() {
    struct sockaddr_in clientAddress;
    socklen_t inAddrLen = sizeof(clientAddress);
    memset(&clientAddress, 0, sizeof(clientAddress));
    int descriptor = 0;

    while (descriptor = ::accept(conn, (struct sockaddr*)&clientAddress, &inAddrLen)) {
        if (descriptor <= 0) {
            continue;
        }
    
        if (runningClients > config->maxConnections) {
            char clientIP[50];
            inet_ntop(PF_INET, (struct in_addr*)&(clientAddress.sin_addr.s_addr), clientIP, sizeof(clientIP)-1);

            LOG(ERROR) << "Too many connections, allowed limit exhausted!, Rejected client: " << clientIP << ", port: " << ntohs(clientAddress.sin_port);
            sendResponse(descriptor, INVALID_TOO_MANY_CONNECTIONS_JSON);
            close(descriptor);
            continue;
        }

        runningClients++;

        Thread t(&SequencerTask::run, SequencerTask(), descriptor, clientAddress);
        t.detach();
    }
}

// listenNow
int SeqListener::listenNow() {
    if (listening) {
        return 0;
    }

    start();

    listening = true;

    listenThread();
}
