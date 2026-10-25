#include "common_protocol_receiver.h"

#include <stdexcept>
#include <vector>

#include <arpa/inet.h>

ProtocolReceiver::ProtocolReceiver(Socket& socket, PlayerId expected_player_id):
        socket(socket),
        expected_player_id(expected_player_id),
        receivers{{CommandType::Move, &ProtocolReceiver::receive_move_command}} {
    validate_player_id(expected_player_id);
}

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

uint32_t ProtocolReceiver::receive_uint32() {
    uint32_t network_value = 0;
    if (socket.recvall(&network_value, sizeof(network_value)) != sizeof(network_value)) {
        throw std::runtime_error("La conexion se cerro al recibir un entero de 32 bits.");
    }
    return ntohl(network_value);
}

std::string ProtocolReceiver::receive_string() {
    const uint16_t size = receive_uint16();
    std::vector<char> value(size);
    if (size > 0 && socket.recvall(value.data(), size) != static_cast<int>(size)) {
        throw std::runtime_error("La conexion se cerro al recibir una cadena.");
    }
    return std::string(value.begin(), value.end());
}

void ProtocolReceiver::validate_player_id(PlayerId player_id) {
    if (player_id == 0 || player_id > MAX_PLAYER_ID) {
        throw std::invalid_argument("El identificador de jugador debe estar entre 1 y 4.");
    }
}

Direction ProtocolReceiver::direction_from_byte(uint8_t value) {
    if (value > static_cast<uint8_t>(Direction::Right)) {
        throw std::invalid_argument("La direccion recibida no es valida.");
    }
    return static_cast<Direction>(value);
}

Command ProtocolReceiver::receive_move_command() {
    const PlayerId player_id = receive_uint8();
    validate_player_id(player_id);
    if (player_id != expected_player_id) {
        throw std::invalid_argument("El comando no pertenece al jugador de la conexion.");
    }

    const uint8_t x = receive_uint8();
    const uint8_t y = receive_uint8();
    const Direction direction = direction_from_byte(receive_uint8());
    return Command(player_id, CommandType::Move, x, y, direction);
}

std::optional<Command> ProtocolReceiver::receive_command() {
    const std::optional<uint8_t> type_byte = receive_uint8_or_end();
    if (!type_byte.has_value()) {
        return std::nullopt;
    }

    const CommandType type = static_cast<CommandType>(type_byte.value());
    const auto receiver = receivers.find(type);
    if (receiver == receivers.end()) {
        throw std::invalid_argument("El tipo de comando recibido no es valido.");
    }

    return (this->*(receiver->second))();
}
