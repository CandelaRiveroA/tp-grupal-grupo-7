#ifndef COMMON_PROTOCOL_H
#define COMMON_PROTOCOL_H

#include <cstdint>
#include <string>

#include "common_socket.h"

class Protocol {
private:
    Socket socket;

public:
    Protocol(const char* hostname, const char* port);
    explicit Protocol(const char* port);

    Protocol(const Protocol&) = delete;
    Protocol& operator=(const Protocol&) = delete;
    Protocol(Protocol&&) = default;
    Protocol& operator=(Protocol&&) = default;

    void accept_connection();
    void send_message(const std::string& message);
    std::string receive_message();
};

#endif
