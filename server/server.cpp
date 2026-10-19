#include "server.h"

Server::Server(const char* port): protocol(port) {}

void Server::run() {
    protocol.accept_connection();
    protocol.send_message("hola mundo");
}
