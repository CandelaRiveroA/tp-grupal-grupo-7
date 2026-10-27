#ifndef COMMON_PROTOCOL_SENDER_H
#define COMMON_PROTOCOL_SENDER_H

#include <cstdint>
#include <string>

#include "common_command.h"
#include "common_server_status.h"
#include "common_socket.h"

class ProtocolSender {
private:
    Socket& socket;

    void send_uint8(uint8_t value);
    void send_uint16(uint16_t value);
    // cppcheck-suppress unusedPrivateFunction
    void send_string(const std::string& value);

    static void validate_direction(Direction direction);

    void send_message(const protocol::MoveMessage& message);

public:
    explicit ProtocolSender(Socket& socket);

    void send_client_message(const protocol::ClientMessage& message);
    void send_server_status(const ServerStatus& status);
    void send_welcome(const ServerWelcome& welcome);
};

#endif
