#ifndef SERVER_H
#define SERVER_H

#include "../common/common_protocol.h"

class Server {
private:
    Protocol protocol;

public:
    explicit Server(const char* port);

    void run();
};

#endif
