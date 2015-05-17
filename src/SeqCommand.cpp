/* ---------------------------------------------------------------------------
** see LICENSE.md
**
** SeqCommand.cpp
**
** Author: rnarmala
** -------------------------------------------------------------------------*/
#ifndef _SEQUENCE_CMD
#define _SEQUENCE_CMD

// Note: update this in INVALID_CMD_KEY_JSON as well
#define MAX_KEY_LENGTH 64

#define INVALID_CMD_JSON "{\"success\": 0, \"error\": \"Invalid request.\"}"

#define INVALID_CMD_KEY_EMPTY_JSON "{\"success\": 0, \"error\": \"Please provide a key.\"}"

#define INVALID_CMD_KEY_JSON "{\"success\": 0, \"error\": \"Invalid Key, Key must be alpha numeric and should start with a letter And length must be atleast 1 and must not exceed 64.\"}"

#define INVALID_CMD_NO_KEY_JSON "{\"success\": 0, \"error\": \"key does not exist.\"}"

#define INVALID_CMD_ALREADY_KEY_JSON "{\"success\": 0, \"error\": \"key already exist.\"}"

#define INVALID_CMD_KEY_VAL_INVALID_JSON "{\"success\": 0, \"error\": \"Inavalid value, value cannot be empty and must be a positive integer.\"}"

#define INVALID_CMD_UNAUTH_JSON "{\"success\": 0, \"error\": \"Authentication failed.\"}"
#define INVALID_TOO_MANY_CONNECTIONS_JSON "{\"success\": 0, \"error\": \"Too many connections.\"}"

#define SUCCESS_JSON_BEGIN "{\"success\": 1, \"data\": "

#define SUCCESS_JSON_END "}"

#define _OP_ID "op"
#define _KEY_ID "key"
#define _KEY_VAL "value"

#define _CMD_STATUS "status"
#define _CMD_GET "get"
#define _CMD_CREATE "create"
#define _CMD_SET "set"
#define _CMD_UUID "uuid"

#define READ_BYTES 256

// send response to client
int sendResponse(int& descriptor, const char* resp) {
    if (send(descriptor, resp, strlen(resp), MSG_NOSIGNAL | MSG_DONTWAIT | MSG_EOR ) == -1) {
        LOG(ERROR) << "Could not send Response " << stderr << ", Response: " << resp;
        return 0;
    }

    return 1;
}

void closeClientConnection(int& descriptor) {
    //close(descriptor);
}

// running on a Thread
void processTask(const char* cmdStr, int cmdStrLen, int descriptor) {
    StringMap parsed;
    parsed.reserve(100);
    parseString(cmdStr, parsed);

    if (!parsed.count(_OP_ID)) {
        sendResponse(descriptor, INVALID_CMD_JSON);
        closeClientConnection(descriptor);
        return;
    }

    if (parsed[_OP_ID] == _CMD_STATUS) {

        time_t timer = time(NULL);
        string tt (ctime(&timer));
        tt.erase(std::find_if(tt.rbegin(), tt.rend(), std::not1(std::ptr_fun<int, int>(std::isspace))).base(), tt.end());
        string resp;
        resp.append(SUCCESS_JSON_BEGIN).append("\"").append(tt).append("\"").append(SUCCESS_JSON_END);

        sendResponse(descriptor, resp.c_str());
        closeClientConnection(descriptor);
        return;

    } else if (parsed[_OP_ID] == _CMD_GET || parsed[_OP_ID] == _CMD_SET || parsed[_OP_ID] == _CMD_CREATE) {
        if (!parsed.count(_KEY_ID)) {
            sendResponse(descriptor, INVALID_CMD_KEY_EMPTY_JSON);
            closeClientConnection(descriptor);
            return;
        }

        if (!isAlphaId(parsed[_KEY_ID].c_str()) || parsed[_KEY_ID].length() == 0 || parsed[_KEY_ID].length() > MAX_KEY_LENGTH) {
            sendResponse(descriptor, INVALID_CMD_KEY_JSON);
            closeClientConnection(descriptor);
            return;
        }

        if (parsed[_OP_ID] == _CMD_GET) {
            if (!counters.count(parsed[_KEY_ID])) {
                sendResponse(descriptor, INVALID_CMD_NO_KEY_JSON);
                closeClientConnection(descriptor);
                return;
            }

            string resp;
            resp.append(SUCCESS_JSON_BEGIN).append("\"");

            // get next value
            resp.append(std::to_string(++counters[parsed[_KEY_ID]]->value));

            resp.append("\"").append(SUCCESS_JSON_END);

            sendResponse(descriptor, resp.c_str());
            closeClientConnection(descriptor);
            return;
        } else if (parsed[_OP_ID] == _CMD_SET) {
            if (!parsed.count(_KEY_VAL) || !isNumber(parsed[_KEY_VAL].c_str())) {
                sendResponse(descriptor, INVALID_CMD_KEY_VAL_INVALID_JSON);
                closeClientConnection(descriptor);
                return;
            }
            
            if (!counters.count(parsed[_KEY_ID])) {
                sendResponse(descriptor, INVALID_CMD_NO_KEY_JSON);
                closeClientConnection(descriptor);
                return;
            }
            
            std::string::size_type sz = 0;
            uHugeInt val = std::stoll(parsed[_KEY_VAL], &sz, 10);
            
            counters[parsed[_KEY_ID]]->value = val;

            string resp;
            resp.append(SUCCESS_JSON_BEGIN).append("\"").append(parsed[_KEY_VAL]).append("\"").append(SUCCESS_JSON_END);

            sendResponse(descriptor, resp.c_str());
            closeClientConnection(descriptor);
            return;
        }  else if (parsed[_OP_ID] == _CMD_CREATE) {
            if (!parsed.count(_KEY_VAL) || !isNumber(parsed[_KEY_VAL].c_str())) {
                sendResponse(descriptor, INVALID_CMD_KEY_VAL_INVALID_JSON);
                closeClientConnection(descriptor);
                return;
            }
            
            //std::cout << "counter size: " << counters.size() << "\n";
            //for ( auto it = counters.begin(); it != counters.end(); ++it )
                //std::cout << "counter: " << it->first << ":" << it->second->value << "\n";
            
            if (counters.count(parsed[_KEY_ID])) {
                sendResponse(descriptor, INVALID_CMD_ALREADY_KEY_JSON);
                closeClientConnection(descriptor);
                return;
            }

            std::string::size_type sz = 0;
            uHugeInt val = std::stoll(parsed[_KEY_VAL], &sz, 10);
            addNewSequence(parsed[_KEY_ID], val);

            string resp;
            resp.append(SUCCESS_JSON_BEGIN).append("\"").append(parsed[_KEY_VAL]).append("\"").append(SUCCESS_JSON_END);

            sendResponse(descriptor, resp.c_str());
            closeClientConnection(descriptor);
            return;
        }
    } else if (parsed[_OP_ID] == _CMD_UUID) {
            char uuid[50];
            getUUID(uuid);

            string resp;
            resp.append(SUCCESS_JSON_BEGIN).append("\"").append(uuid).append("\"").append(SUCCESS_JSON_END);

            sendResponse(descriptor, resp.c_str());
            closeClientConnection(descriptor);
            return;
    }
}




// Sequencer Task
struct SequencerTask {
    SequencerTask() {
    }

    SequencerTask(const SequencerTask&) {
    }

    // run this task
    void run(int descriptor, struct sockaddr_in clientAddress) {
        if (config.debug) {
            char clientIP[50];

            inet_ntop(PF_INET, (struct in_addr*)&(clientAddress.sin_addr.s_addr), clientIP, sizeof(clientIP)-1);
            int clientPort = ntohs(clientAddress.sin_port);

            //printf("\nReceived request from Client: %s:%d\n", clientIP, clientPort);
            LOG(INFO) << "Received request from Client: " << clientIP << ", port: " << clientPort;
        }

        // Read Bytes
        char buffer[READ_BYTES];
        ssize_t len;

        while ((len = read(descriptor, buffer, sizeof(buffer))) > 0) {
            buffer[len] = 0;

            //printf("processRequest - %s\n", request);
            LOG(INFO) << "processRequest: " << buffer;

            Thread t(&processTask, buffer, len, descriptor);
            t.join();
        }

        // connection closed.
        runningClients--;
    }
};


#endif