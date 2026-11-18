#ifndef COMMON_PROTOCOL_RECEIVER_H
#define COMMON_PROTOCOL_RECEIVER_H

#include <cstdint>
#include <map>
#include <optional>
#include <string>

#include "common_command.h"
#include "common_match_request.h"
#include "common_server_status.h"
#include "common_socket.h"

class ProtocolReceiver {
private:
    Socket& socket;
    using Handler = protocol::ClientMessage (ProtocolReceiver::*)();
    std::map<protocol::ClientMessageType, Handler> handlers;
    using ServerHandler = ServerMessage (ProtocolReceiver::*)();
    std::map<ServerMessageType, ServerHandler> server_handlers;

    std::optional<uint8_t> receive_uint8_or_end();
    uint8_t receive_uint8();
    uint16_t receive_uint16();
    std::string receive_string();
    // Inverso de ProtocolSender::send_appearance: mismo orden, 8 bytes.
    CharacterAppearance receive_appearance();
    protocol::ClientMessage receive_move_message();
    protocol::ClientMessage receive_pick_message();
    protocol::ClientMessage receive_take_message();
    protocol::ClientMessage receive_sleep_message();
    protocol::ClientMessage receive_wake_message();
    protocol::ClientMessage receive_chat_message();
    ServerMessage receive_status_message();
    ServerMessage receive_leave_message();
    ServerMessage receive_resource_message();
    ServerMessage receive_dropped_message();
    ServerMessage receive_inventory_slot_message();
    ServerMessage receive_obtained_message();
    ServerMessage receive_tool_used_message();
    ServerMessage receive_resource_hit_message();
    ServerMessage receive_reset_message();
    ServerMessage receive_sync_end_message();
    ServerMessage receive_animal_message();
    ServerMessage receive_enemy_message();
    ServerMessage receive_projectile_message();
    ServerMessage receive_server_sleep_message();
    ServerMessage receive_furniture_message();
    ServerMessage receive_day_end_message();
    ServerMessage receive_day_message();
    ServerMessage receive_sleeping_players_message();
    ServerMessage receive_clock_message();
    ServerMessage receive_crop_message();
    ServerMessage receive_ground_effect_message();
    ServerMessage receive_server_chat_message();

    static Direction direction_from_byte(uint8_t value);
    static ResourceDamage damage_from_byte(uint8_t value);

public:
    explicit ProtocolReceiver(Socket& socket);

    std::optional<protocol::ClientMessage> receive_command();
    ServerMessage receive_server_message();
    ServerWelcome receive_welcome();
    MatchRequest receive_match_request();
    LobbyStatus receive_lobby_status();
};

#endif
