#include "common_protocol_receiver.h"

#include <stdexcept>
#include <utility>
#include <vector>

#include <arpa/inet.h>

ProtocolReceiver::ProtocolReceiver(SocketIO& socket):
        socket(socket),
        handlers{
                {protocol::ClientMessageType::SelectSlot,
                 &ProtocolReceiver::receive_select_slot_message},
                {protocol::ClientMessageType::Move, &ProtocolReceiver::receive_move_message},
                {protocol::ClientMessageType::Pick, &ProtocolReceiver::receive_pick_message},
                {protocol::ClientMessageType::Take, &ProtocolReceiver::receive_take_message},
                {protocol::ClientMessageType::Sleep, &ProtocolReceiver::receive_sleep_message},
                {protocol::ClientMessageType::Wake, &ProtocolReceiver::receive_wake_message},
                {protocol::ClientMessageType::Chat, &ProtocolReceiver::receive_chat_message},
                {protocol::ClientMessageType::GiftResponse,
                 &ProtocolReceiver::receive_gift_response_message},
                {protocol::ClientMessageType::ToggleWalking,
                 &ProtocolReceiver::receive_toggle_walking_message},
        },
        server_handlers{
                {ServerMessageType::PlayerHeld, &ProtocolReceiver::receive_player_held_message},
                {ServerMessageType::GiftAccepted, &ProtocolReceiver::receive_gift_accepted_message},
                {ServerMessageType::Status, &ProtocolReceiver::receive_status_message},
                {ServerMessageType::Leave, &ProtocolReceiver::receive_leave_message},
                {ServerMessageType::Resource, &ProtocolReceiver::receive_resource_message},
                {ServerMessageType::Dropped, &ProtocolReceiver::receive_dropped_message},
                {ServerMessageType::InventorySlot,
                 &ProtocolReceiver::receive_inventory_slot_message},
                {ServerMessageType::Obtained, &ProtocolReceiver::receive_obtained_message},
                {ServerMessageType::ToolUsed, &ProtocolReceiver::receive_tool_used_message},
                {ServerMessageType::ResourceHit, &ProtocolReceiver::receive_resource_hit_message},
                {ServerMessageType::Reset, &ProtocolReceiver::receive_reset_message},
                {ServerMessageType::SyncEnd, &ProtocolReceiver::receive_sync_end_message},
                {ServerMessageType::Animal, &ProtocolReceiver::receive_animal_message},
                {ServerMessageType::Enemy, &ProtocolReceiver::receive_enemy_message},
                {ServerMessageType::Projectile, &ProtocolReceiver::receive_projectile_message},
                {ServerMessageType::Sleep, &ProtocolReceiver::receive_server_sleep_message},
                {ServerMessageType::Furniture, &ProtocolReceiver::receive_furniture_message},
                {ServerMessageType::DayEnd, &ProtocolReceiver::receive_day_end_message},
                {ServerMessageType::Day, &ProtocolReceiver::receive_day_message},
                {ServerMessageType::SleepingPlayers,
                 &ProtocolReceiver::receive_sleeping_players_message},
                {ServerMessageType::Clock, &ProtocolReceiver::receive_clock_message},
                {ServerMessageType::Crop, &ProtocolReceiver::receive_crop_message},
                {ServerMessageType::GroundEffect, &ProtocolReceiver::receive_ground_effect_message},
                {ServerMessageType::Chat, &ProtocolReceiver::receive_server_chat_message},
                {ServerMessageType::GiftRequest, &ProtocolReceiver::receive_gift_request_message},
                {ServerMessageType::GiftAnswered, &ProtocolReceiver::receive_gift_answered_message},
                {ServerMessageType::SetCompleted, &ProtocolReceiver::receive_set_completed_message},
                {ServerMessageType::Victory, &ProtocolReceiver::receive_victory_message},
                {ServerMessageType::Gold, &ProtocolReceiver::receive_gold_message},
                {ServerMessageType::NpcPlacement,
                 &ProtocolReceiver::receive_npc_placement_message},
                {ServerMessageType::MapChange,
                 &ProtocolReceiver::receive_map_change_message},
                {ServerMessageType::MarketOffer,
                 &ProtocolReceiver::receive_market_offer_message},
        } {}

std::optional<uint8_t> ProtocolReceiver::receive_uint8_or_end() {
    uint8_t value = 0;
    const int received = socket.recvall(&value, sizeof(value));
    if (received == 0) {
        return std::nullopt;
    }
    return value;
}

uint8_t ProtocolReceiver::receive_uint8() {
    const std::optional<uint8_t> value = receive_uint8_or_end();
    if (!value.has_value()) {
        throw std::runtime_error("La conexion se cerro durante el mensaje.");
    }
    return value.value();
}

uint16_t ProtocolReceiver::receive_uint16() {
    uint16_t network_value = 0;
    if (socket.recvall(&network_value, sizeof(network_value)) != sizeof(network_value)) {
        throw std::runtime_error("La conexion se cerro al recibir un entero de 16 bits.");
    }
    return ntohs(network_value);
}

std::string ProtocolReceiver::receive_string() {
    const uint16_t size = receive_uint16();
    std::vector<char> value(size);
    if (size > 0 && socket.recvall(value.data(), size) != static_cast<int>(size)) {
        throw std::runtime_error("La conexion se cerro al recibir una cadena.");
    }
    return std::string(value.begin(), value.end());
}

CharacterAppearance ProtocolReceiver::receive_appearance() {
    CharacterAppearance appearance;
    appearance.skin = receive_uint8();
    appearance.eyes = receive_uint8();
    appearance.hair = receive_uint8();
    appearance.hairColor = receive_uint8();
    appearance.shirt = receive_uint8();
    appearance.pants = receive_uint8();
    appearance.pantsColor = receive_uint8();
    appearance.accessory = receive_uint8();
    return appearance;
}

Direction ProtocolReceiver::direction_from_byte(uint8_t value) {
    if (value > static_cast<uint8_t>(Direction::RIGHT)) {
        throw std::invalid_argument("La direccion recibida no es valida.");
    }
    return static_cast<Direction>(value);
}

ResourceDamage ProtocolReceiver::damage_from_byte(uint8_t value) {
    if (value > static_cast<uint8_t>(ResourceDamage::Broken)) {
        throw std::invalid_argument("El estado de daño recibido no es valido.");
    }
    return static_cast<ResourceDamage>(value);
}

protocol::ClientMessage ProtocolReceiver::receive_move_message() {
    const Direction direction = direction_from_byte(receive_uint8());
    return protocol::MoveMessage{direction};
}

protocol::ClientMessage ProtocolReceiver::receive_pick_message() {
    const Direction direction = direction_from_byte(receive_uint8());
    const uint8_t slot = receive_uint8();
    return protocol::PickMessage{direction, slot};
}

protocol::ClientMessage ProtocolReceiver::receive_take_message() {
    const Direction direction = direction_from_byte(receive_uint8());
    const uint8_t slot = receive_uint8();
    return protocol::TakeMessage{direction, slot};
}

protocol::ClientMessage ProtocolReceiver::receive_sleep_message() {
    const uint8_t accept = receive_uint8();
    if (accept > 1) {
        throw std::invalid_argument("El indicador de aceptacion del sueno no es valido.");
    }
    return protocol::SleepMessage{accept == 1};
}

protocol::ClientMessage ProtocolReceiver::receive_wake_message() { return protocol::WakeMessage{}; }

protocol::ClientMessage ProtocolReceiver::receive_chat_message() {
    return protocol::ChatMessage{receive_string()};
}

protocol::ClientMessage ProtocolReceiver::receive_toggle_walking_message() {
    return protocol::ToggleWalkingMessage{};
}

std::optional<protocol::ClientMessage> ProtocolReceiver::receive_command() {
    const std::optional<uint8_t> type_byte = receive_uint8_or_end();
    if (!type_byte.has_value()) {
        return std::nullopt;
    }

    const auto type = static_cast<protocol::ClientMessageType>(type_byte.value());
    const auto handler = handlers.find(type);
    if (handler == handlers.end()) {
        throw std::invalid_argument("El tipo de comando recibido no es valido.");
    }

    return (this->*(handler->second))();
}

ServerMessage ProtocolReceiver::receive_status_message() {
    const uint8_t player_id = receive_uint8();
    const uint16_t x = receive_uint16();
    const uint16_t y = receive_uint16();
    const Direction direction = direction_from_byte(receive_uint8());
    const CharacterAppearance appearance = receive_appearance();
    const uint16_t health = receive_uint16();
    const uint16_t max_health = receive_uint16();
    const uint16_t energy = receive_uint16();
    const uint16_t max_energy = receive_uint16();
    const uint8_t fainted = receive_uint8();
    return ServerStatus(player_id, x, y, direction, appearance, health, max_health, energy, max_energy, fainted);
}

ServerMessage ProtocolReceiver::receive_leave_message() { return ServerLeave{receive_uint8()}; }

ServerMessage ProtocolReceiver::receive_resource_message() {
    const uint16_t x = receive_uint16();
    const uint16_t y = receive_uint16();
    std::string content_id = receive_string();
    const ResourceDamage damage = damage_from_byte(receive_uint8());
    return ServerResource{x, y, std::move(content_id), damage};
}

ServerMessage ProtocolReceiver::receive_dropped_message() {
    const uint16_t x = receive_uint16();
    const uint16_t y = receive_uint16();
    std::string content_id = receive_string();
    const uint8_t interactive = receive_uint8();
    if (interactive > 1) {
        throw std::invalid_argument("El indicador de interactividad no es valido.");
    }
    return ServerDropped{x, y, std::move(content_id), interactive == 1};
}

ServerMessage ProtocolReceiver::receive_inventory_slot_message() {
    const uint8_t slot = receive_uint8();
    const uint8_t count = receive_uint8();
    std::string content_id = receive_string();
    const uint8_t tradable = receive_uint8();
    if (tradable > 1) {
        throw std::invalid_argument("El indicador de transferibilidad no es valido.");
    }
    return ServerInventorySlot{slot, count, std::move(content_id), tradable == 1};
}

ServerMessage ProtocolReceiver::receive_obtained_message() {
    return ServerObtained{receive_string()};
}

ServerMessage ProtocolReceiver::receive_resource_hit_message() {
    const uint16_t x = receive_uint16();
    const uint16_t y = receive_uint16();
    const uint8_t destroyed = receive_uint8();
    if (destroyed > 1) {
        throw std::invalid_argument("El indicador de destruccion no es valido.");
    }
    return ServerResourceHit{x, y, destroyed == 1};
}

ServerMessage ProtocolReceiver::receive_tool_used_message() {
    const uint8_t player_id = receive_uint8();
    return ServerToolUsed{player_id, receive_string()};
}

ServerMessage ProtocolReceiver::receive_reset_message() { return ServerReset{}; }

ServerMessage ProtocolReceiver::receive_sync_end_message() { return ServerSyncEnd{}; }

ServerMessage ProtocolReceiver::receive_animal_message() {
    const uint8_t id = receive_uint8();
    std::string kind = receive_string();
    const uint16_t x = receive_uint16();
    const uint16_t y = receive_uint16();
    const Direction direction = direction_from_byte(receive_uint8());
    const uint8_t ready = receive_uint8();
    if (ready > 1) {
        throw std::invalid_argument("El indicador de ordeñe no es valido.");
    }
    return ServerAnimal{id, std::move(kind), x, y, direction, ready == 1};
}

ServerMessage ProtocolReceiver::receive_enemy_message() {
    const uint16_t id = receive_uint16();
    std::string kind = receive_string();
    const uint16_t x = receive_uint16();
    const uint16_t y = receive_uint16();
    const Direction direction = direction_from_byte(receive_uint8());
    const uint8_t health = receive_uint8();
    const uint8_t action = receive_uint8();
    if (action > static_cast<uint8_t>(EnemyAction::Damage)) {
        throw std::invalid_argument("La accion del enemigo no es valida.");
    }
    return ServerEnemy{id, std::move(kind), x, y, direction, health,
                       static_cast<EnemyAction>(action)};
}

ServerMessage ProtocolReceiver::receive_projectile_message() {
    const uint16_t id = receive_uint16();
    std::string kind = receive_string();
    const uint16_t x = receive_uint16();
    const uint16_t y = receive_uint16();
    const Direction direction = direction_from_byte(receive_uint8());
    return ServerProjectile{id, std::move(kind), x, y, direction};
}

ServerMessage ProtocolReceiver::receive_server_sleep_message() {
    const uint8_t sleeping = receive_uint8();
    const uint8_t total = receive_uint8();
    return ServerSleep{sleeping, total};
}

ServerMessage ProtocolReceiver::receive_furniture_message() {
    const uint16_t x = receive_uint16();
    const uint16_t y = receive_uint16();
    std::string content_id = receive_string();
    return ServerFurniture{x, y, std::move(content_id)};
}

ServerMessage ProtocolReceiver::receive_day_end_message() { return ServerDayEnd{}; }

ServerMessage ProtocolReceiver::receive_day_message() { return ServerDay{}; }

ServerMessage ProtocolReceiver::receive_sleeping_players_message() {
    const uint8_t count = receive_uint8();
    std::vector<uint8_t> players(count);
    for (uint8_t& id: players) {
        id = receive_uint8();
    }
    return ServerSleepingPlayers{std::move(players)};
}

ServerMessage ProtocolReceiver::receive_clock_message() {
    const uint8_t day = receive_uint8();
    const uint16_t minutes = receive_uint16();
    return ServerClock{day, minutes};
}

ServerMessage ProtocolReceiver::receive_crop_message() {
    const uint16_t x = receive_uint16();
    const uint16_t y = receive_uint16();
    std::string crop_id = receive_string();
    const uint8_t stage = receive_uint8();
    const uint8_t needs_water = receive_uint8();
    const uint8_t interactive = receive_uint8();
    const uint8_t quality = receive_uint8();
    if (needs_water > 1 || interactive > 1) {
        throw std::invalid_argument("Los indicadores del cultivo no son validos.");
    }
    if (quality < 1 || quality > 5) {
        throw std::invalid_argument("La calidad del cultivo no es valida.");
    }
    return ServerCrop{x, y, std::move(crop_id), stage, needs_water == 1, interactive == 1, quality};
}

ServerMessage ProtocolReceiver::receive_ground_effect_message() {
    const uint16_t x = receive_uint16();
    const uint16_t y = receive_uint16();
    const uint8_t effect = receive_uint8();
    if (effect > 2) {
        throw std::invalid_argument("El efecto de suelo no es valido.");
    }
    return ServerGroundEffect{x, y, static_cast<GroundEffect>(effect)};
}

ServerMessage ProtocolReceiver::receive_server_chat_message() {
    const uint8_t player_id = receive_uint8();
    const std::string characterName = receive_string();
    const std::string text = receive_string();
    const uint8_t system = receive_uint8();
    return ServerChat{player_id, characterName, text, system == 1};
}

ServerMessage ProtocolReceiver::receive_set_completed_message() {
    return ServerSetCompleted{receive_uint8()};
}

ServerMessage ProtocolReceiver::receive_gold_message() {
    return ServerGold{receive_uint16()};
}

ServerMessage ProtocolReceiver::receive_npc_placement_message() {
    std::string id = receive_string();
    const uint16_t x = receive_uint16();
    const uint16_t y = receive_uint16();
    return ServerNpcPlacement{std::move(id), x, y};
}

ServerMessage ProtocolReceiver::receive_map_change_message() {
    return ServerMapChange{receive_string()};
}

ServerMessage ProtocolReceiver::receive_market_offer_message() {
    std::string merchant_id = receive_string();
    const uint16_t count = receive_uint16();
    std::vector<ServerMarketEntry> entries;
    entries.reserve(count);
    for (uint16_t i = 0; i < count; ++i) {
        std::string content_id = receive_string();
        std::string label = receive_string();
        const uint8_t side = receive_uint8();
        if (side > static_cast<uint8_t>(ServerMarketSide::Buys)) {
            throw std::invalid_argument("El lado de la oferta de mercado no es valido.");
        }
        const uint8_t has_price = receive_uint8();
        if (has_price > 1) {
            throw std::invalid_argument("El indicador de precio del mercado no es valido.");
        }
        const uint16_t price = receive_uint16();
        entries.push_back(ServerMarketEntry{std::move(content_id),
                                            std::move(label),
                                            static_cast<ServerMarketSide>(side),
                                            has_price == 1, price});
    }
    return ServerMarketOffer{std::move(merchant_id), std::move(entries)};
}

ServerMessage ProtocolReceiver::receive_victory_message() {
    return ServerVictory{};
}

ServerMessage ProtocolReceiver::receive_server_message() {
    const ServerMessageType type = static_cast<ServerMessageType>(receive_uint8());
    const auto handler = server_handlers.find(type);
    if (handler == server_handlers.end()) {
        throw std::invalid_argument("El tipo de mensaje del servidor no es valido.");
    }

    return (this->*(handler->second))();
}

ServerWelcome ProtocolReceiver::receive_welcome() {
    const ServerMessageType type = static_cast<ServerMessageType>(receive_uint8());
    if (type != ServerMessageType::Welcome) {
        throw std::invalid_argument("Se esperaba un mensaje de bienvenida.");
    }

    return ServerWelcome{receive_uint8(), receive_string()};
}

MatchRequest ProtocolReceiver::receive_match_request() {
    const uint8_t type = receive_uint8();
    if (type == static_cast<uint8_t>(MatchRequestType::ListWorlds)) {
        return MatchRequest{MatchRequestType::ListWorlds, {}, {}, {}, {}, {},
                            CharacterAppearance{}};
    }
    if (type != static_cast<uint8_t>(MatchRequestType::Create) &&
        type != static_cast<uint8_t>(MatchRequestType::Join)) {
        throw std::invalid_argument("El tipo de pedido del lobby no es valido.");
    }
    std::string codeMatch = receive_string();
    std::string nameMatch = receive_string();
    std::string worldName = receive_string();
    std::string username = receive_string();
    std::string characterName = receive_string();
    const CharacterAppearance appearance = receive_appearance();
    return MatchRequest{static_cast<MatchRequestType>(type), std::move(codeMatch),
                        std::move(nameMatch), std::move(worldName), std::move(username),
                        std::move(characterName), appearance};
}

LobbyStatus ProtocolReceiver::receive_lobby_status() {
    const uint8_t status = receive_uint8();
    if (status > static_cast<uint8_t>(LobbyStatus::CharacterNameTaken)) {
        throw std::invalid_argument("El estado del lobby no es valido.");
    }
    return static_cast<LobbyStatus>(status);
}

std::vector<std::string> ProtocolReceiver::receive_world_list() {
    const uint16_t count = receive_uint16();
    std::vector<std::string> worlds;
    worlds.reserve(count);
    for (uint16_t i = 0; i < count; ++i) {
        worlds.push_back(receive_string());
    }
    return worlds;
}

protocol::ClientMessage ProtocolReceiver::receive_gift_response_message() {
    const uint8_t sender = receive_uint8();
    const uint8_t accept = receive_uint8();
    if (accept > 1) {
        throw std::invalid_argument("El indicador de aceptacion de regalo no es valido.");
    }
    return protocol::GiftResponseMessage{sender, accept == 1};
}

ServerMessage ProtocolReceiver::receive_gift_request_message() {
    const uint8_t sender = receive_uint8();
    std::string name = receive_string();
    std::string content = receive_string();
    return ServerGiftRequest{sender, std::move(name), std::move(content)};
}

ServerMessage ProtocolReceiver::receive_gift_answered_message() { return ServerGiftAnswered{}; }


protocol::ClientMessage ProtocolReceiver::receive_select_slot_message() {
    return protocol::SelectSlotMessage{receive_uint8()};
}

ServerMessage ProtocolReceiver::receive_player_held_message() {
    const uint8_t player = receive_uint8();
    return ServerPlayerHeld{player, receive_string()};
}

ServerMessage ProtocolReceiver::receive_gift_accepted_message() {
    return ServerGiftAccepted{receive_uint8()};
}
