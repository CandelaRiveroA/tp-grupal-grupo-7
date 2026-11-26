#ifndef COMMON_SOCKET_IO_H
#define COMMON_SOCKET_IO_H

// Operaciones de transporte que necesita el protocolo.
class SocketIO {
public:
    virtual int sendall(const void* data, unsigned int size) = 0;
    virtual int recvall(void* data, unsigned int size) = 0;
    virtual ~SocketIO() = default;
};

#endif
