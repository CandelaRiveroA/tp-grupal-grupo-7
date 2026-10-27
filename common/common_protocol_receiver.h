#ifndef COMMON_PROTOCOL_RECEIVER_H
#define COMMON_PROTOCOL_RECEIVER_H

#include <cstdint>
#include <map>
#include <optional>
#include <string>

#include "common_command.h"
#include "common_server_status.h"
#include "common_socket.h"

class ProtocolReceiver {
private:
    Socket& socket;
    using Handler = protocol::ClientMessage (ProtocolReceiver::*)();
    std::map<protocol::ClientMessageType, Handler> handlers;

    std::optional<uint8_t> receive_uint8_or_end();
    uint8_t receive_uint8();
    uint16_t receive_uint16();
    std::string receive_string();
    protocol::ClientMessage receive_move_message();

    static Direction direction_from_byte(uint8_t value);

public:
    explicit ProtocolReceiver(Socket& socket);

    std::optional<protocol::ClientMessage> receive_command();
    ServerStatus receive_server_status();
    ServerWelcome receive_welcome();
};

#endif
