#include "matches_monitor.h"

#include <utility>

MatchesMonitor::MatchesMonitor(ServerOutputMonitor& output): output(output) {}

MatchSeat MatchesMonitor::joinLocked(MatchEntry& entry,
                                     const std::shared_ptr<Queue<ServerMessage>>& statuses) {
    if (entry.players >= MAX_JUGADORES) {
        return MatchSeat{LobbyStatus::Full, 0, nullptr};
    }
    const uint8_t player_id = entry.next_player_id;
    try {
        entry.events.push(MatchEvent(PlayerJoined{player_id, statuses}));
    } catch (const ClosedQueue&) {
        // El match ya terminó por un error: no admite jugadores.
        return MatchSeat{LobbyStatus::NotFound, 0, nullptr};
    }
    ++entry.players;
    ++entry.next_player_id;
    return MatchSeat{LobbyStatus::Ok, player_id, &entry.events};
}

MatchSeat MatchesMonitor::create(const std::string& name,
                                 const std::shared_ptr<Queue<ServerMessage>>& statuses) {
    std::lock_guard<std::mutex> lock(mtx);
    if (name.empty()) {
        return MatchSeat{LobbyStatus::Invalid, 0, nullptr};
    }
    if (matches.count(name) > 0) {
        return MatchSeat{LobbyStatus::NameTaken, 0, nullptr};
    }

    auto entry = std::make_unique<MatchEntry>(output);
    MatchEntry& created = *entry;
    matches.emplace(name, std::move(entry));
    created.match.start();
    return joinLocked(created, statuses);
}

MatchSeat MatchesMonitor::join(const std::string& name,
                               const std::shared_ptr<Queue<ServerMessage>>& statuses) {
    std::lock_guard<std::mutex> lock(mtx);
    if (name.empty()) {
        return MatchSeat{LobbyStatus::Invalid, 0, nullptr};
    }
    const auto it = matches.find(name);
    if (it == matches.end()) {
        return MatchSeat{LobbyStatus::NotFound, 0, nullptr};
    }
    return joinLocked(*it->second, statuses);
}

void MatchesMonitor::leave(const std::string& name, uint8_t player_id) {
    std::lock_guard<std::mutex> lock(mtx);
    const auto it = matches.find(name);
    if (it == matches.end()) {
        return;
    }

    MatchEntry& entry = *it->second;
    try {
        entry.events.push(MatchEvent(PlayerLeft{player_id}));
    } catch (const ClosedQueue&) {
        // El match ya terminó: sus colas de salida ya están cerradas.
    }
    --entry.players;
    if (entry.players > 0) {
        return;
    }

    entry.events.close();
    entry.match.join();
    matches.erase(it);
}

void MatchesMonitor::closeAll() {
    std::lock_guard<std::mutex> lock(mtx);
    for (auto& [name, entry]: matches) {
        entry->events.close();
        entry->match.join();
    }
    matches.clear();
}
