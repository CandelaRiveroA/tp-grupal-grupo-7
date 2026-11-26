#include "mock_socket.h"

#include <cstring>
#include <stdexcept>

void MockSocket::set_input(const std::vector<uint8_t>& bytes) {
    input = bytes;
    offset = 0;
}

const std::vector<uint8_t>& MockSocket::sent_bytes() const { return output; }

int MockSocket::sendall(const void* data, unsigned int size) {
    if (size == 0) {
        return 0;
    }
    const auto* bytes = static_cast<const uint8_t*>(data);
    output.insert(output.end(), bytes, bytes + size);
    return static_cast<int>(size);
}

int MockSocket::recvall(void* data, unsigned int size) {
    if (size == 0 || offset == input.size()) {
        return 0;
    }
    if (size > input.size() - offset) {
        offset = input.size();
        throw std::runtime_error("Conexion simulada cerrada durante recvall");
    }
    std::memcpy(data, input.data() + offset, size);
    offset += size;
    return static_cast<int>(size);
}
