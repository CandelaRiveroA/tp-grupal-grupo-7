#ifndef PROTOCOL_MOCK_SOCKET_H
#define PROTOCOL_MOCK_SOCKET_H

#include <cstddef>
#include <cstdint>
#include <vector>

#include "common/common_socket_io.h"

// Sustituto en memoria: no abre conexiones ni utiliza puertos.
class MockSocket: public SocketIO {
private:
    std::vector<uint8_t> input;
    std::vector<uint8_t> output;
    std::size_t offset = 0;

public:
    void set_input(const std::vector<uint8_t>& bytes);
    const std::vector<uint8_t>& sent_bytes() const;
    int sendall(const void* data, unsigned int size) override;
    int recvall(void* data, unsigned int size) override;
};

#endif
