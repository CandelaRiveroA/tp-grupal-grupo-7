#include "server_output_monitor.h"

#include <iostream>

void ServerOutputMonitor::printError(const char* prefix, const char* description) {
    std::lock_guard<std::mutex> lock(mtx);
    std::cerr << prefix << description << std::endl;
}
