#include "common_protocol_sender.h"

#include <limits>
#include <stdexcept>

#include <arpa/inet.h>

ProtocolSender::ProtocolSender(Socket& socket): socket(socket) {}

void ProtocolSender::send_uint8(uint8_t value) {
    if (socket.sendall(&value, sizeof(value)) != sizeof(value)) {
        throw std::runtime_error("No se pudo enviar un entero de 8 bits.");
    }
}

void ProtocolSender::send_uint16(uint16_t value) {
    const uint16_t network_value = htons(value);
    if (socket.sendall(&network_value, sizeof(network_value)) != sizeof(network_value)) {
        throw std::runtime_error("No se pudo enviar un entero de 16 bits.");
    }
}

void ProtocolSender::send_string(const std::string& value) {
    if (value.size() > std::numeric_limits<uint16_t>::max()) {
        throw std::length_error("La cadena del protocolo es demasiado larga.");
    }

    send_uint16(static_cast<uint16_t>(value.size()));
    if (!value.empty() && socket.sendall(value.data(), static_cast<unsigned int>(value.size())) !=
                                  static_cast<int>(value.size())) {
        throw std::runtime_error("No se pudo enviar la cadena del protocolo.");
    }
}

void ProtocolSender::validate_direction(Direction direction) {
    const uint8_t raw_direction = static_cast<uint8_t>(direction);
    if (raw_direction > static_cast<uint8_t>(Direction::RIGHT)) {
        throw std::invalid_argument("La direccion del mensaje no es valida.");
    }
}

void ProtocolSender::send_message(const protocol::MoveMessage& message) {
    validate_direction(message.direction);

    send_uint8(static_cast<uint8_t>(protocol::ClientMessageType::Move));
    send_uint8(static_cast<uint8_t>(message.direction));
}

void ProtocolSender::send_message(const protocol::PickMessage& message) {
    validate_direction(message.direction);

    send_uint8(static_cast<uint8_t>(protocol::ClientMessageType::Pick));
    send_uint8(static_cast<uint8_t>(message.direction));
}

void ProtocolSender::send_message(const protocol::TakeMessage& message) {
    validate_direction(message.direction);

    send_uint8(static_cast<uint8_t>(protocol::ClientMessageType::Take));
    send_uint8(static_cast<uint8_t>(message.direction));
}

void ProtocolSender::send_message(const protocol::SleepMessage& message) {
    send_uint8(static_cast<uint8_t>(protocol::ClientMessageType::Sleep));
    send_uint8(message.accept ? 1 : 0);
}

void ProtocolSender::send_client_message(const protocol::ClientMessage& message) {
    std::visit([this](const auto& alternative) { send_message(alternative); }, message);
}

void ProtocolSender::send_message(const ServerStatus& status) {
    validate_direction(status.direction);

    send_uint8(static_cast<uint8_t>(ServerMessageType::Status));
    send_uint8(status.player_id);
    send_uint16(status.x);
    send_uint16(status.y);
    send_uint8(static_cast<uint8_t>(status.direction));
}

void ProtocolSender::send_message(const ServerLeave& leave) {
    send_uint8(static_cast<uint8_t>(ServerMessageType::Leave));
    send_uint8(leave.player_id);
}

void ProtocolSender::send_message(const ServerResource& resource) {
    send_uint8(static_cast<uint8_t>(ServerMessageType::Resource));
    send_uint16(resource.x);
    send_uint16(resource.y);
    send_string(resource.content_id);
    send_uint8(static_cast<uint8_t>(resource.damage));
}

void ProtocolSender::send_message(const ServerDropped& dropped) {
    send_uint8(static_cast<uint8_t>(ServerMessageType::Dropped));
    send_uint16(dropped.x);
    send_uint16(dropped.y);
    send_string(dropped.content_id);
    send_uint8(dropped.interactive ? 1 : 0);
}

void ProtocolSender::send_message(const ServerInventorySlot& slot) {
    send_uint8(static_cast<uint8_t>(ServerMessageType::InventorySlot));
    send_uint8(slot.slot);
    send_uint8(slot.count);
    send_string(slot.content_id);
}

void ProtocolSender::send_message(const ServerObtained& obtained) {
    send_uint8(static_cast<uint8_t>(ServerMessageType::Obtained));
    send_string(obtained.content_id);
}

void ProtocolSender::send_message(const ServerToolUsed& used) {
    send_uint8(static_cast<uint8_t>(ServerMessageType::ToolUsed));
    send_uint8(used.player_id);
}

void ProtocolSender::send_message(const ServerResourceHit& hit) {
    send_uint8(static_cast<uint8_t>(ServerMessageType::ResourceHit));
    send_uint16(hit.x);
    send_uint16(hit.y);
    send_uint8(hit.destroyed ? 1 : 0);
}

void ProtocolSender::send_message(const ServerReset& /*reset*/) {
    send_uint8(static_cast<uint8_t>(ServerMessageType::Reset));
}

void ProtocolSender::send_message(const ServerAnimal& animal) {
    validate_direction(animal.direction);

    send_uint8(static_cast<uint8_t>(ServerMessageType::Animal));
    send_uint8(animal.id);
    send_string(animal.kind);
    send_uint16(animal.x);
    send_uint16(animal.y);
    send_uint8(static_cast<uint8_t>(animal.direction));
    send_uint8(animal.ready_to_milk ? 1 : 0);
}

void ProtocolSender::send_message(const ServerSleep& sleep) {
    send_uint8(static_cast<uint8_t>(ServerMessageType::Sleep));
    send_uint8(sleep.sleeping);
    send_uint8(sleep.total);
}

void ProtocolSender::send_message(const ServerFurniture& furniture) {
    send_uint8(static_cast<uint8_t>(ServerMessageType::Furniture));
    send_uint16(furniture.x);
    send_uint16(furniture.y);
    send_string(furniture.content_id);
}

void ProtocolSender::send_message(const ServerDay& /*day*/) {
    send_uint8(static_cast<uint8_t>(ServerMessageType::Day));
}

void ProtocolSender::send_message(const ServerSleepingPlayers& sleepers) {
    send_uint8(static_cast<uint8_t>(ServerMessageType::SleepingPlayers));
    send_uint8(static_cast<uint8_t>(sleepers.players.size()));
    for (const uint8_t id: sleepers.players) {
        send_uint8(id);
    }
}

void ProtocolSender::send_message(const ServerClock& clock) {
    send_uint8(static_cast<uint8_t>(ServerMessageType::Clock));
    send_uint16(clock.minutes);
}

void ProtocolSender::send_server_message(const ServerMessage& message) {
    std::visit([this](const auto& alternative) { send_message(alternative); }, message);
}

void ProtocolSender::send_welcome(const ServerWelcome& welcome) {
    send_uint8(static_cast<uint8_t>(ServerMessageType::Welcome));
    send_uint8(welcome.player_id);
}

void ProtocolSender::send_match_request(const MatchRequest& request) {
    send_uint8(static_cast<uint8_t>(request.type));
    send_string(request.codeMatch);
    send_string(request.nameMatch);
    send_string(request.username);
    send_string(request.characterName);
}

void ProtocolSender::send_lobby_status(LobbyStatus status) {
    send_uint8(static_cast<uint8_t>(status));
}
