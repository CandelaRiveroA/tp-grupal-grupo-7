#ifndef COMMON_MATCH_REQUEST_H
#define COMMON_MATCH_REQUEST_H

#include <cstdint>
#include <string>

// Primer mensaje de cada conexion del lobby: crear o unirse a un match.
enum class MatchRequestType : uint8_t { Create = 1, Join = 2 };

struct MatchRequest {
    MatchRequestType type;
    std::string name;
};

// Respuesta al pedido del lobby. Si es Ok, la conexion queda como jugador del match.
enum class LobbyStatus : uint8_t { Ok = 0, NameTaken = 1, NotFound = 2, Full = 3, Invalid = 4 };

#endif
