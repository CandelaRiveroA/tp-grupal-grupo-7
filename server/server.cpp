#include "server.h"

#include <iostream>
#include <string>

Server::Server(const char* service): match(events, output), acceptor(service, events, output) {}

void Server::run() {
    match.start();
    acceptor.start();

    std::string line;
    while (std::getline(std::cin, line)) {
        if (line == "q") {
            break;
        }
    }

    acceptor.stop();
    acceptor.join();

    events.close();
    match.join();
}
