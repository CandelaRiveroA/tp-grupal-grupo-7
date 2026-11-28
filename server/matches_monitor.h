#ifndef MATCHES_MONITOR_H
#define MATCHES_MONITOR_H

#ifndef MAX_JUGADORES
#define MAX_JUGADORES 4
#endif

#include <cstdint>
#include <map>
#include <memory>
#include <mutex>
#include <optional>
#include <string>
#include <vector>

#include "common_match_request.h"
#include "common/model/draft/WorldSummary.h"
#include "common_queue.h"
#include "common_server_status.h"
#include "match.h"
#include "world_model/World.h"
#include "server_output_monitor.h"

struct MatchSeat {
    LobbyStatus status;
    uint8_t player_id;
    Queue<MatchEvent>* events;
    std::string worldName;
};

class MatchesMonitor {
private:
    struct MatchEntry {
        Queue<MatchEvent> events;
        Match match;
        uint8_t players = 0;
        uint8_t next_player_id = 1;
        uint8_t capacity = MAX_JUGADORES;
        std::string worldName;
        // TODO persistencia: el username identifica al usuario para sus partidas guardadas.
        std::map<uint8_t, std::string> usernames;
        std::map<uint8_t, std::string> characters;

        explicit MatchEntry(ServerOutputMonitor& output, worldmodel::World&& world):
                match(events, output, std::move(world)) {}
    };

    ServerOutputMonitor& output;
    std::string worldsDir;
    std::mutex mtx;
    std::map<std::string, std::unique_ptr<MatchEntry>> matches;

    uint8_t capacityFor(const worldmodel::World& world) const;
    std::optional<LobbyStatus> identityError(const std::string& username,
                                             const std::string& characterName) const;
    MatchSeat joinLocked(MatchEntry& entry, const std::string& username,
                         const std::string& characterName,
                         const std::shared_ptr<Queue<ServerMessage>>& statuses,
                         const CharacterAppearance& appearance);

public:
    explicit MatchesMonitor(ServerOutputMonitor& output,
                            const std::string& worldsDir = TALLER_WORLDS_DIR);

    MatchesMonitor(const MatchesMonitor&) = delete;
    MatchesMonitor& operator=(const MatchesMonitor&) = delete;

    // La apariencia se guarda tal cual la manda el cliente (sin validar). Va
    // con default para no romper a los llamados que hoy no la mandan.
    MatchSeat create(const std::string& codeMatch, const std::string& worldName,
                     const std::string& username,
                     const std::string& characterName,
                     const std::shared_ptr<Queue<ServerMessage>>& statuses,
                     const CharacterAppearance& appearance = CharacterAppearance{});
    MatchSeat join(const std::string& codeMatch, const std::string& username,
                   const std::string& characterName,
                   const std::shared_ptr<Queue<ServerMessage>>& statuses,
                   const CharacterAppearance& appearance = CharacterAppearance{});
    std::vector<std::string> validWorldNames() const;
    std::vector<WorldSummary> validWorldSummaries() const;
    void leave(const std::string& codeMatch, uint8_t player_id);
    void closeAll();
};

#endif
