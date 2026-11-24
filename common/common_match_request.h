#ifndef COMMON_MATCH_REQUEST_H
#define COMMON_MATCH_REQUEST_H

#include <cstdint>
#include <string>

#include "common/model/draft/CharacterAppearance.h"

// Primer mensaje de cada conexion del lobby: crear o unirse a un match.
enum class MatchRequestType : uint8_t { Create = 1, Join = 2, ListWorlds = 3 };

#define kMaxUsernameLength 32
#define kMaxCharacterNameLength 24

struct MatchRequest {
    MatchRequestType type;
    std::string codeMatch;
    std::string nameMatch;
    std::string worldName;
    std::string username;
    std::string characterName;
    // Personalizacion del personaje: indices de capa tal cual los eligio el
    // cliente, sin validar (el server no conoce los counts de characters.toml).
    CharacterAppearance appearance;
};

// Respuesta al pedido del lobby. Si es Ok, la conexion queda como jugador del match.
enum class LobbyStatus : uint8_t {
    Ok = 0,
    NameTaken = 1,
    NotFound = 2,
    Full = 3,
    Invalid = 4,
    UsernameTaken = 5,
    CharacterNameTaken = 6
};

#endif
