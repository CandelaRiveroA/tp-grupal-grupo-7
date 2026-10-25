#ifndef COMMON_PROTOCOL_RECEIVER_H
#define COMMON_PROTOCOL_RECEIVER_H

#include <cstdint>
#include <map>
#include <optional>
#include <string>

#include "common_command.h"
#include "common_socket.h"

class ProtocolReceiver {
private:
    Socket& socket;
    PlayerId expected_player_id;
    using Receiver = Command (ProtocolReceiver::*)();
    std::map<CommandType, Receiver> receivers;

    std::optional<uint8_t> receive_uint8_or_end();
    uint8_t receive_uint8();
    uint16_t receive_uint16();
    // cppcheck-suppress unusedPrivateFunction
    uint32_t receive_uint32();
    // cppcheck-suppress unusedPrivateFunction
    std::string receive_string();
    Command receive_move_command();

    static void validate_player_id(PlayerId player_id);
    static Direction direction_from_byte(uint8_t value);

public:
    ProtocolReceiver(Socket& socket, PlayerId expected_player_id);

    std::optional<Command> receive_command();
};

#endif
