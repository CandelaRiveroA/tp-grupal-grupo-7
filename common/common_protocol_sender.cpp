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

void ProtocolSender::send_server_message(const ServerMessage& message) {
    std::visit([this](const auto& alternative) { send_message(alternative); }, message);
}

void ProtocolSender::send_welcome(const ServerWelcome& welcome) {
    send_uint8(static_cast<uint8_t>(ServerMessageType::Welcome));
    send_uint8(welcome.player_id);
}
