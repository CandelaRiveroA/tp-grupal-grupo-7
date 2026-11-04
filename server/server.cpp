#include "server.h"

#include <iostream>
#include <string>

Server::Server(const char* service): matches(output), acceptor(service, matches, output) {}

void Server::run() {
    acceptor.start();

    std::string line;
    while (std::getline(std::cin, line)) {
        if (line == "q") {
            break;
        }
    }

    acceptor.stop();
    acceptor.join();

    matches.closeAll();
}
