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
        // TODO persistencia: el username identifica al usuario para sus partidas guardadas.
        std::map<uint8_t, std::string> usernames;
        std::map<uint8_t, std::string> characters;

        explicit MatchEntry(ServerOutputMonitor& output): match(events, output) {}
    };

    ServerOutputMonitor& output;
    std::mutex mtx;
    std::map<std::string, std::unique_ptr<MatchEntry>> matches;

    std::optional<LobbyStatus> identityError(const std::string& username,
                                             const std::string& characterName) const;
    MatchSeat joinLocked(MatchEntry& entry, const std::string& username,
                         const std::string& characterName,
                         const std::shared_ptr<Queue<ServerMessage>>& statuses,
                         const CharacterAppearance& appearance);

public:
    explicit MatchesMonitor(ServerOutputMonitor& output);

    MatchesMonitor(const MatchesMonitor&) = delete;
    MatchesMonitor& operator=(const MatchesMonitor&) = delete;

    // La apariencia se guarda tal cual la manda el cliente (sin validar). Va
    // con default para no romper a los llamados que hoy no la mandan.
    MatchSeat create(const std::string& codeMatch, const std::string& username,
                     const std::string& characterName,
                     const std::shared_ptr<Queue<ServerMessage>>& statuses,
                     const CharacterAppearance& appearance = CharacterAppearance{});
    MatchSeat join(const std::string& codeMatch, const std::string& username,
                   const std::string& characterName,
                   const std::shared_ptr<Queue<ServerMessage>>& statuses,
                   const CharacterAppearance& appearance = CharacterAppearance{});
    void leave(const std::string& codeMatch, uint8_t player_id);
    void closeAll();
};

#endif
