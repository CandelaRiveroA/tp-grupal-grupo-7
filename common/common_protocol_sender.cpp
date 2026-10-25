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

void ProtocolSender::send_uint32(uint32_t value) {
    const uint32_t network_value = htonl(value);
    if (socket.sendall(&network_value, sizeof(network_value)) != sizeof(network_value)) {
        throw std::runtime_error("No se pudo enviar un entero de 32 bits.");
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

void ProtocolSender::validate_player_id(PlayerId player_id) {
    if (player_id == 0 || player_id > MAX_PLAYER_ID) {
        throw std::invalid_argument("El identificador de jugador debe estar entre 1 y 4.");
    }
}

void ProtocolSender::validate_direction(Direction direction) {
    const uint8_t raw_direction = static_cast<uint8_t>(direction);
    if (raw_direction > static_cast<uint8_t>(Direction::Right)) {
        throw std::invalid_argument("La direccion del comando no es valida.");
    }
}

void ProtocolSender::send_move_command(PlayerId player_id, uint8_t x, uint8_t y,
                                       Direction direction) {
    validate_player_id(player_id);
    validate_direction(direction);

    send_uint8(static_cast<uint8_t>(CommandType::Move));
    send_uint8(player_id);
    send_uint8(x);
    send_uint8(y);
    send_uint8(static_cast<uint8_t>(direction));
}
