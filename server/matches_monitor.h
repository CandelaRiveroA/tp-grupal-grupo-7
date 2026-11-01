#ifndef MATCHES_MONITOR_H
#define MATCHES_MONITOR_H

#ifndef MAX_JUGADORES
#define MAX_JUGADORES 4
#endif

#include <cstdint>
#include <map>
#include <memory>
#include <mutex>
#include <string>

#include "common_match_request.h"
#include "common_queue.h"
#include "common_server_status.h"
#include "match.h"
#include "server_output_monitor.h"

struct MatchSeat {
    LobbyStatus status;
    uint8_t player_id;
    Queue<MatchEvent>* events;
};

// shortcut: el codigo de union es el nombre del match, sin expiracion ni
// generacion (cualquiera que lo conozca entra), subir a codigo generado y
// expirable cuando el lobby lo pida.
class MatchesMonitor {
private:
    struct MatchEntry {
        Queue<MatchEvent> events;
        Match match;
        uint8_t players = 0;
        uint8_t next_player_id = 1;

        explicit MatchEntry(ServerOutputMonitor& output): match(events, output) {}
    };

    ServerOutputMonitor& output;
    std::mutex mtx;
    std::map<std::string, std::unique_ptr<MatchEntry>> matches;

    MatchSeat joinLocked(MatchEntry& entry, const std::shared_ptr<Queue<ServerMessage>>& statuses);

public:
    explicit MatchesMonitor(ServerOutputMonitor& output);

    MatchesMonitor(const MatchesMonitor&) = delete;
    MatchesMonitor& operator=(const MatchesMonitor&) = delete;

    MatchSeat create(const std::string& name,
                     const std::shared_ptr<Queue<ServerMessage>>& statuses);
    MatchSeat join(const std::string& name, const std::shared_ptr<Queue<ServerMessage>>& statuses);
    void leave(const std::string& name, uint8_t player_id);
    void closeAll();
};

#endif
