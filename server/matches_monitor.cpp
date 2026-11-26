#include "matches_monitor.h"

#include "world_model/FormatError.h"
#include "world_model/MapRules.h"
#include "world_model/MapObject.h"
#include "world_model/WorldRepository.h"
#include "world_model/WorldValidator.h"

#include <algorithm>
#include <utility>

MatchesMonitor::MatchesMonitor(ServerOutputMonitor& output, const std::string& worldsDir):
        output(output), worldsDir(worldsDir) {}

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
                                     const std::shared_ptr<Queue<ServerMessage>>& statuses,
                                     const CharacterAppearance& appearance) {
    for (const auto& [id, taken]: entry.characters) {
        if (taken == characterName) {
            return MatchSeat{LobbyStatus::CharacterNameTaken, 0, nullptr, entry.worldName};
        }
    }
    if (entry.players >= entry.capacity) {
        return MatchSeat{LobbyStatus::Full, 0, nullptr, entry.worldName};
    }
    const uint8_t player_id = entry.next_player_id;
    try {
        entry.events.push(MatchEvent(PlayerJoined{player_id, statuses, appearance, characterName}));
    } catch (const ClosedQueue&) {
        // El match ya terminó por un error: no admite jugadores.
        return MatchSeat{LobbyStatus::NotFound, 0, nullptr, entry.worldName};
    }
    entry.usernames[player_id] = username;
    entry.characters[player_id] = characterName;
    ++entry.players;
    ++entry.next_player_id;
    return MatchSeat{LobbyStatus::Ok, player_id, &entry.events, entry.worldName};
}

MatchSeat MatchesMonitor::create(const std::string& codeMatch, const std::string& worldName,
                                const std::string& username,
                                const std::string& characterName,
                                const std::shared_ptr<Queue<ServerMessage>>& statuses,
                                const CharacterAppearance& appearance) {
    if (const std::optional<LobbyStatus> error = identityError(username, characterName)) {
        return MatchSeat{*error, 0, nullptr, {}};
    }
    if (codeMatch.empty() || !worldmodel::MapRules().isValidWorldName(worldName)) {
        return MatchSeat{LobbyStatus::Invalid, 0, nullptr, {}};
    }

    worldmodel::World world;
    uint8_t capacity;
    try {
        const worldmodel::WorldRepository repo(worldsDir);
        world = repo.load(worldName);
        const std::vector<worldmodel::ValidationIssue> issues =
                worldmodel::WorldValidator().validate(world);
        if (!issues.empty()) {
            return MatchSeat{LobbyStatus::Invalid, 0, nullptr, {}};
        }
        int cabins = 0;
        const auto farmIt = world.maps.find("farm");
        if (farmIt != world.maps.end()) {
            for (const worldmodel::MapObject& o : farmIt->second.objects) {
                if (o.role == worldmodel::ObjectRole::Cabin) {
                    ++cabins;
                }
            }
        }
        if (cabins > 0 && cabins <= MAX_JUGADORES) {
            capacity = static_cast<uint8_t>(cabins);
        }
    } catch (const std::exception&) {
        return MatchSeat{LobbyStatus::Invalid, 0, nullptr, {}};
    }

    std::lock_guard<std::mutex> lock(mtx);
    if (matches.count(codeMatch) > 0) {
        return MatchSeat{LobbyStatus::NameTaken, 0, nullptr, {}};
    }

    auto entry = std::make_unique<MatchEntry>(output, std::move(world));
    MatchEntry& created = *entry;
    created.worldName = worldName;
    created.capacity = capacity;
    matches.emplace(codeMatch, std::move(entry));
    created.match.start();
    return joinLocked(created, username, characterName, statuses, appearance);
}

MatchSeat MatchesMonitor::join(const std::string& codeMatch, const std::string& username,
                               const std::string& characterName,
                               const std::shared_ptr<Queue<ServerMessage>>& statuses,
                               const CharacterAppearance& appearance) {
    std::lock_guard<std::mutex> lock(mtx);
    if (const std::optional<LobbyStatus> error = identityError(username, characterName)) {
        return MatchSeat{*error, 0, nullptr, {}};
    }
    if (codeMatch.empty()) {
        return MatchSeat{LobbyStatus::Invalid, 0, nullptr, {}};
    }
    const auto it = matches.find(codeMatch);
    if (it == matches.end()) {
        return MatchSeat{LobbyStatus::NotFound, 0, nullptr, {}};
    }
    MatchSeat seat = joinLocked(*it->second, username, characterName, statuses, appearance);
    seat.worldName = it->second->worldName;
    return seat;
}

std::vector<std::string> MatchesMonitor::validWorldNames() const {
    const worldmodel::WorldRepository repository(worldsDir);
    const worldmodel::WorldValidator validator;
    const worldmodel::MapRules rules;
    std::vector<std::string> result;
    for (const std::string& name : repository.listWorlds()) {
        if (!rules.isValidWorldName(name) ||
            std::find(result.begin(), result.end(), name) != result.end()) {
            continue;
        }
        try {
            const worldmodel::World world = repository.load(name);
            if (validator.validate(world).empty()) {
                result.push_back(world.name);
            }
        } catch (const worldmodel::FormatError&) {
            // Los archivos ilegibles o corruptos no se ofrecen en el lobby.
        }
    }
    return result;
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