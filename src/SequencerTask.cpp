/* ---------------------------------------------------------------------------
** see LICENSE.md
**
** SequencerTask.cpp
**     Sequencer Task Object
**
** Author: rnarmala
** -------------------------------------------------------------------------*/

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

    _CMD cmd = UNKNOWN;
    string cmdText = parsed[_OP_ID];

    if (cmdText == _STATUS_TEXT) {
        cmd = STATUS;
    } else if (cmdText == _NEXT_TEXT) {
        cmd = NEXT;
    } else if (cmdText == _GET_TEXT) {
        cmd = GET;
    } else if (cmdText == _CREATE_TEXT) {
        cmd = CREATE;
    } else if (cmdText == _SET_TEXT) {
        cmd = SET;
    } else if (cmdText == _UUID_TEXT) {
        cmd = UUID;
    } else {
        sendResponse(descriptor, UNKNOWN_CMD_JSON);
        closeClientConnection(descriptor);
        return;
    }


    switch (cmd)
    {
        case STATUS:
        {
            time_t timer = time(NULL);
            string tt (ctime(&timer));
            tt.erase(std::find_if(tt.rbegin(), tt.rend(), std::not1(std::ptr_fun<int, int>(std::isspace))).base(), tt.end());
            string resp;
            resp.append(SUCCESS_JSON_BEGIN).append("\"").append(tt).append("\"").append(SUCCESS_JSON_END);

            sendResponse(descriptor, resp.c_str());
            closeClientConnection(descriptor);
            return;
        }
        break;

        case NEXT:
        case SET:
        case CREATE:
        case GET:
        {
            if (!parsed.count(_KEY_ID)) {
                sendResponse(descriptor, INVALID_CMD_KEY_EMPTY_JSON);
                closeClientConnection(descriptor);
                return;
            }

            string key = parsed[_KEY_ID];

            if (!isAlphaId(key.c_str()) || key.length() == 0 || key.length() > MAX_KEY_LENGTH) {
                sendResponse(descriptor, INVALID_CMD_KEY_JSON);
                closeClientConnection(descriptor);
                return;
            }

            switch (cmd)
            {
                case NEXT:
                {
                    if (!counters.count(key)) {
                        sendResponse(descriptor, INVALID_CMD_NO_KEY_JSON);
                        closeClientConnection(descriptor);
                        return;
                    }

                    string resp;
                    resp.append(SUCCESS_JSON_BEGIN).append("\"");

                    // get next value
                    resp.append(std::to_string(++counters[key]->value));

                    resp.append("\"").append(SUCCESS_JSON_END);

                    sendResponse(descriptor, resp.c_str());
                    closeClientConnection(descriptor);
                    return;
                }
                break;

                case GET:
                {
                    if (!counters.count(key)) {
                        sendResponse(descriptor, INVALID_CMD_NO_KEY_JSON);
                        closeClientConnection(descriptor);
                        return;
                    }

                    string resp;
                    resp.append(SUCCESS_JSON_BEGIN).append("\"");

                    // get next value
                    resp.append(std::to_string(counters[key]->value));

                    resp.append("\"").append(SUCCESS_JSON_END);

                    sendResponse(descriptor, resp.c_str());
                    closeClientConnection(descriptor);
                    return;
                }
                break;

                case SET:
                {
                    if (!parsed.count(_KEY_VAL) || !isNumber(parsed[_KEY_VAL].c_str())) {
                        sendResponse(descriptor, INVALID_CMD_KEY_VAL_INVALID_JSON);
                        closeClientConnection(descriptor);
                        return;
                    }

                    if (!counters.count(key)) {
                        sendResponse(descriptor, INVALID_CMD_NO_KEY_JSON);
                        closeClientConnection(descriptor);
                        return;
                    }

                    string value = parsed[_KEY_VAL];
                    std::string::size_type sz = 0;
                    uHugeInt val = std::stoll(value, &sz, 10);

                    counters[key]->value = val;

                    string resp;
                    resp.append(SUCCESS_JSON_BEGIN).append("\"").append(value).append("\"").append(SUCCESS_JSON_END);

                    sendResponse(descriptor, resp.c_str());
                    closeClientConnection(descriptor);
                    return;
                }
                break;

                case CREATE:
                {
                    if (!parsed.count(_KEY_VAL) || !isNumber(parsed[_KEY_VAL].c_str())) {
                        sendResponse(descriptor, INVALID_CMD_KEY_VAL_INVALID_JSON);
                        closeClientConnection(descriptor);
                        return;
                    }

                    string value = parsed[_KEY_VAL];

                    //std::cout << "counter size: " << counters.size() << "\n";
                    //for ( auto it = counters.begin(); it != counters.end(); ++it )
                        //std::cout << "counter: " << it->first << ":" << it->second->value << "\n";

                    if (counters.count(key)) {
                        sendResponse(descriptor, INVALID_CMD_ALREADY_KEY_JSON);
                        closeClientConnection(descriptor);
                        return;
                    }

                    std::string::size_type sz = 0;
                    uHugeInt val = std::stoll(value, &sz, 10);
                    addNewSequence(key, val);

                    string resp;
                    resp.append(SUCCESS_JSON_BEGIN).append("\"").append(value).append("\"").append(SUCCESS_JSON_END);

                    sendResponse(descriptor, resp.c_str());
                    closeClientConnection(descriptor);
                    return;
                }
                break;
            } // switch ends here

        }
        break;

        case UUID:
        {
            char uuid[50];
            getUUID(uuid);

            string resp;
            resp.append(SUCCESS_JSON_BEGIN).append("\"").append(uuid).append("\"").append(SUCCESS_JSON_END);

            sendResponse(descriptor, resp.c_str());
            closeClientConnection(descriptor);
            return;
        }
        break;

    } // switch ends here
}




// Sequencer Task
SequencerTask::SequencerTask() {
}

SequencerTask::SequencerTask(const SequencerTask&) {
}

// run this task
void SequencerTask::run(int descriptor, struct sockaddr_in clientAddress) {
    if (config->debug) {
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
