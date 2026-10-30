#include "common_protocol_receiver.h"

#include <stdexcept>
#include <utility>
#include <vector>

#include <arpa/inet.h>

ProtocolReceiver::ProtocolReceiver(Socket& socket):
        socket(socket),
        handlers{
                {protocol::ClientMessageType::Move, &ProtocolReceiver::receive_move_message},
                {protocol::ClientMessageType::Pick, &ProtocolReceiver::receive_pick_message},
                {protocol::ClientMessageType::Take, &ProtocolReceiver::receive_take_message},
        },
        server_handlers{
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
    return protocol::PickMessage{direction};
}

protocol::ClientMessage ProtocolReceiver::receive_take_message() {
    const Direction direction = direction_from_byte(receive_uint8());
    return protocol::TakeMessage{direction};
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
    return ServerStatus(player_id, x, y, direction);
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
    return ServerDropped{x, y, std::move(content_id)};
}

ServerMessage ProtocolReceiver::receive_inventory_slot_message() {
    const uint8_t slot = receive_uint8();
    const uint8_t count = receive_uint8();
    std::string content_id = receive_string();
    return ServerInventorySlot{slot, count, std::move(content_id)};
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
    return ServerToolUsed{receive_uint8()};
}

ServerMessage ProtocolReceiver::receive_reset_message() { return ServerReset{}; }

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

    return ServerWelcome{receive_uint8()};
}
