#ifndef COMMON_PROTOCOL_SENDER_H
#define COMMON_PROTOCOL_SENDER_H

#include <cstdint>
#include <string>

#include "common_command.h"
#include "common_match_request.h"
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
    void send_message(const protocol::PickMessage& message);
    void send_message(const protocol::TakeMessage& message);
    void send_message(const protocol::SleepMessage& message);
    void send_message(const ServerStatus& status);
    void send_message(const ServerLeave& leave);
    void send_message(const ServerResource& resource);
    void send_message(const ServerDropped& dropped);
    void send_message(const ServerInventorySlot& slot);
    void send_message(const ServerObtained& obtained);
    void send_message(const ServerToolUsed& used);
    void send_message(const ServerResourceHit& hit);
    void send_message(const ServerReset& reset);
    void send_message(const ServerAnimal& animal);
    void send_message(const ServerSleep& sleep);
    void send_message(const ServerFurniture& furniture);
    void send_message(const ServerDay& day);
    void send_message(const ServerSleepingPlayers& sleepers);
    void send_message(const ServerClock& clock);

public:
    explicit ProtocolSender(Socket& socket);

    void send_client_message(const protocol::ClientMessage& message);
    void send_server_message(const ServerMessage& message);
    void send_welcome(const ServerWelcome& welcome);
    void send_match_request(const MatchRequest& request);
    void send_lobby_status(LobbyStatus status);
};

#endif
