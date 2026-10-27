#ifndef SERVER_OUTPUT_MONITOR_H
#define SERVER_OUTPUT_MONITOR_H

#include <mutex>

class ServerOutputMonitor {
private:
    std::mutex mtx;

public:
    ServerOutputMonitor() = default;

    ServerOutputMonitor(const ServerOutputMonitor&) = delete;
    ServerOutputMonitor& operator=(const ServerOutputMonitor&) = delete;

    void printError(const char* prefix, const char* description);
};

#endif
