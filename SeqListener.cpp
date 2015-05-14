// he backlog argument defines the maximum length to which
// the queue of pending connections for sockfd may grow.
// If a connection request arrives when the queue is full,
// the client may receive an error with an indication of ECONNREFUSED
#define LISTEN_BACKLOG 50

#define READ_BYTES 256

// SeqListener
class SeqListener {
protected:
    int port;
    string host;
    int conn;
    bool listening;

    // open socket, bind and then listen
    int start() {
        conn = socket(PF_INET, SOCK_STREAM, 0);

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
            perror("Socket bind() failed");
            return success;
        }

        success = listen(conn, LISTEN_BACKLOG);

        if (success != 0) {
            perror("Socket listen() failed");
            return success;
        }

        return success;
    }

public:
    SeqListener(int p) {
        port = p;
    }

    ~SeqListener() {
    }

    int listenNow() {
        if (listening) {
            return 0;
        }

        start();
        listening = true;

        while(1) {
            struct sockaddr_in clientAddress;
            socklen_t inAddrLen = sizeof(clientAddress);
            memset(&clientAddress, 0, sizeof(clientAddress));

            int descriptor = ::accept(conn, (struct sockaddr*)&clientAddress, &inAddrLen);
            if (descriptor < 0) {
                perror("Connection from client accept() failed");
                continue;
            }

            char clientIP[50];
            inet_ntop(PF_INET, (struct in_addr*)&(clientAddress.sin_addr.s_addr), clientIP, sizeof(clientIP)-1);
            int clientPort = ntohs(clientAddress.sin_port);

            printf("\nReceived request from Client: %s:%d\n", clientIP, clientPort);

            // Read Bytes
            char buffer[READ_BYTES];
            ssize_t len;

            while ((len = read(descriptor, buffer, sizeof(buffer))) > 0) {
                buffer[len] = 0;

                processRequest(buffer, len, descriptor);
            }
        }
    }

    // Process Request
    int processRequest(const char* request, int requestLen, int& descriptor) {
        printf("processRequest - %s\n", request);

        // send date time to client
        time_t timer = time(NULL);
        if (send(descriptor, ctime(&timer), 30, 0) == -1) {
            perror("send");
        }

        // close client connection, this is needed
        close(descriptor);

        return 1;
    }
};

