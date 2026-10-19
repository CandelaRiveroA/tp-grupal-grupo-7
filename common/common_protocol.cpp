#include "common_protocol.h"

#include <stdexcept>
#include <vector>

#include <arpa/inet.h>

#define MAX_MESSAGE_SIZE 1024

Protocol::Protocol(const char* hostname, const char* port): socket(hostname, port) {}

Protocol::Protocol(const char* port): socket(port) {}

void Protocol::accept_connection() { socket = socket.accept(); }

void Protocol::send_message(const std::string& message) {
    if (message.size() > MAX_MESSAGE_SIZE) {
        throw std::length_error("El mensaje del protocolo es demasiado largo.");
    }

    const uint32_t size = htonl(static_cast<uint32_t>(message.size()));
    socket.sendall(&size, sizeof(size));
    if (!message.empty()) {
        socket.sendall(message.data(), static_cast<unsigned int>(message.size()));
    }
}

std::string Protocol::receive_message() {
    uint32_t network_size = 0;
    if (socket.recvall(&network_size, sizeof(network_size)) == 0) {
        throw std::runtime_error("La conexion fue cerrada antes de recibir el tamaño del mensaje.");
    }

    const uint32_t size = ntohl(network_size);
    if (size > MAX_MESSAGE_SIZE) {
        throw std::length_error("El mensaje del protocolo es demasiado largo.");
    }

    std::vector<char> buffer(size);
    if (size > 0 && socket.recvall(buffer.data(), size) == 0) {
        throw std::runtime_error("La conexion fue cerrada antes de recibir el mensaje.");
    }
    return std::string(buffer.begin(), buffer.end());
}
