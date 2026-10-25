#ifndef COMMON_PROTOCOL_SENDER_H
#define COMMON_PROTOCOL_SENDER_H

#include <cstdint>
#include <string>

#include "common_command_types.h"
#include "common_server_status.h"
#include "common_socket.h"

class ProtocolSender {
private:
    Socket& socket;

    void send_uint8(uint8_t value);
    void send_uint16(uint16_t value);
    // cppcheck-suppress unusedPrivateFunction
    void send_uint32(uint32_t value);
    // cppcheck-suppress unusedPrivateFunction
    void send_string(const std::string& value);

    static void validate_player_id(PlayerId player_id);
    static void validate_direction(Direction direction);

public:
    explicit ProtocolSender(Socket& socket);

    void send_move_command(PlayerId player_id, uint8_t x, uint8_t y, Direction direction);
    void send_server_status(const ServerStatus& status);
};

#endif
