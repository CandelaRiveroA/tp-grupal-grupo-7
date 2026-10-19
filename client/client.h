#ifndef CLIENT_H
#define CLIENT_H

#include "../common/common_protocol.h"

class Client {
private:
    Protocol protocol;

public:
    Client(const char* hostname, const char* port);

    void run();
};

#endif
