#include "client.h"

#include <iostream>

Client::Client(const char* hostname, const char* port): protocol(hostname, port) {}

void Client::run() { std::cout << protocol.receive_message() << std::endl; }
