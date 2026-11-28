#ifndef COMMON_PROTOCOL_RECEIVER_H
#define COMMON_PROTOCOL_RECEIVER_H

#include <cstdint>
#include <map>
#include <optional>
#include <string>
#include <vector>

#include "common_command.h"
#include "common_match_request.h"
#include "common/model/draft/WorldSummary.h"
#include "common_server_status.h"
#include "common_socket_io.h"

class ProtocolReceiver {
private:
    SocketIO& socket;
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
    ServerInventorySlot receive_slot_payload();
    protocol::ClientMessage receive_donate_community_message();
    ServerMessage receive_community_progress_message();
    protocol::ClientMessage receive_discard_message();
    protocol::ClientMessage receive_open_container_message();
    protocol::ClientMessage receive_close_container_message();
    protocol::ClientMessage receive_container_transfer_message();
    protocol::ClientMessage receive_place_furniture_message();
    protocol::ClientMessage receive_grab_furniture_message();
    protocol::ClientMessage receive_move_message();
    protocol::ClientMessage receive_pick_message();
    protocol::ClientMessage receive_take_message();
    protocol::ClientMessage receive_sleep_message();
    protocol::ClientMessage receive_wake_message();
    protocol::ClientMessage receive_chat_message();
    protocol::ClientMessage receive_toggle_walking_message();
    protocol::ClientMessage receive_select_slot_message();
    ServerMessage receive_player_held_message();
    ServerMessage receive_gift_accepted_message();
    ServerMessage receive_player_name_message();
    ServerMessage receive_inventory_full_message();
    ServerMessage receive_world_players_message();
    ServerMessage receive_container_state_message();
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
    ServerMessage receive_gift_request_message();
    ServerMessage receive_gift_answered_message();
    ServerMessage receive_set_completed_message();
    ServerMessage receive_victory_message();
    ServerMessage receive_gold_message();
    ServerMessage receive_npc_placement_message();
    ServerMessage receive_map_change_message();
    ServerMessage receive_market_offer_message();

    static Direction direction_from_byte(uint8_t value);
    static ResourceDamage damage_from_byte(uint8_t value);

public:
    explicit ProtocolReceiver(SocketIO& socket);

    std::optional<protocol::ClientMessage> receive_command();
    protocol::ClientMessage receive_gift_response_message();
    ServerMessage receive_server_message();
    ServerWelcome receive_welcome();
    MatchRequest receive_match_request();
    LobbyStatus receive_lobby_status();
    std::vector<WorldSummary> receive_world_list();
};

#endif
