#include "matches_monitor.h"

#include <utility>

MatchesMonitor::MatchesMonitor(ServerOutputMonitor& output): output(output) {}

std::optional<LobbyStatus> MatchesMonitor::identityError(const std::string& username,
                                                         const std::string& characterName) const {
    // shortcut: el largo se mide en bytes y la comparacion es exacta (distingue mayusculas), subir cuando haya normalizacion de usuarios.
    if (username.empty() || username.size() > kMaxUsernameLength) {
        return LobbyStatus::Invalid;
    }
    if (characterName.empty() || characterName.size() > kMaxCharacterNameLength) {
        return LobbyStatus::Invalid;
    }
    for (const auto& match_entry: matches) {
        for (const auto& [id, taken]: match_entry.second->usernames) {
            if (taken == username) {
                return LobbyStatus::UsernameTaken;
            }
        }
    }
    return std::nullopt;
}

MatchSeat MatchesMonitor::joinLocked(MatchEntry& entry, const std::string& username,
                                     const std::string& characterName,
                                     const std::shared_ptr<Queue<ServerMessage>>& statuses) {
    if (entry.players >= MAX_JUGADORES) {
        return MatchSeat{LobbyStatus::Full, 0, nullptr};
    }
    for (const auto& [id, taken]: entry.characters) {
        if (taken == characterName) {
            return MatchSeat{LobbyStatus::CharacterNameTaken, 0, nullptr};
        }
    }
    const uint8_t player_id = entry.next_player_id;
    try {
        entry.events.push(MatchEvent(PlayerJoined{player_id, statuses}));
    } catch (const ClosedQueue&) {
        // El match ya terminó por un error: no admite jugadores.
        return MatchSeat{LobbyStatus::NotFound, 0, nullptr};
    }
    entry.usernames[player_id] = username;
    entry.characters[player_id] = characterName;
    ++entry.players;
    ++entry.next_player_id;
    return MatchSeat{LobbyStatus::Ok, player_id, &entry.events};
}

MatchSeat MatchesMonitor::create(const std::string& codeMatch, const std::string& username,
                                 const std::string& characterName,
                                 const std::shared_ptr<Queue<ServerMessage>>& statuses) {
    std::lock_guard<std::mutex> lock(mtx);
    if (const std::optional<LobbyStatus> error = identityError(username, characterName)) {
        return MatchSeat{*error, 0, nullptr};
    }
    if (codeMatch.empty()) {
        return MatchSeat{LobbyStatus::Invalid, 0, nullptr};
    }
    if (matches.count(codeMatch) > 0) {
        return MatchSeat{LobbyStatus::NameTaken, 0, nullptr};
    }

    auto entry = std::make_unique<MatchEntry>(output);
    MatchEntry& created = *entry;
    matches.emplace(codeMatch, std::move(entry));
    created.match.start();
    return joinLocked(created, username, characterName, statuses);
}

MatchSeat MatchesMonitor::join(const std::string& codeMatch, const std::string& username,
                               const std::string& characterName,
                               const std::shared_ptr<Queue<ServerMessage>>& statuses) {
    std::lock_guard<std::mutex> lock(mtx);
    if (const std::optional<LobbyStatus> error = identityError(username, characterName)) {
        return MatchSeat{*error, 0, nullptr};
    }
    if (codeMatch.empty()) {
        return MatchSeat{LobbyStatus::Invalid, 0, nullptr};
    }
    const auto it = matches.find(codeMatch);
    if (it == matches.end()) {
        return MatchSeat{LobbyStatus::NotFound, 0, nullptr};
    }
    return joinLocked(*it->second, username, characterName, statuses);
}

void MatchesMonitor::leave(const std::string& codeMatch, uint8_t player_id) {
    std::lock_guard<std::mutex> lock(mtx);
    const auto it = matches.find(codeMatch);
    if (it == matches.end()) {
        return;
    }

    MatchEntry& entry = *it->second;
    entry.usernames.erase(player_id);
    entry.characters.erase(player_id);
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
