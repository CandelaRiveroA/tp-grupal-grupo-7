#ifndef COMMON_SERVER_STATUS_H
#define COMMON_SERVER_STATUS_H

#include <cstdint>
#include <string>
#include <variant>

#include "common_constants.h"

enum class ServerMessageType : uint8_t {
    Status = 1,
    Welcome = 2,
    Leave = 3,
    Resource = 4,
    Dropped = 5,
    InventorySlot = 6,
    Obtained = 7,
    ToolUsed = 8,
    ResourceHit = 9,
    Reset = 10,
};

// Estado de daño visible de un recurso picable.
enum class ResourceDamage : uint8_t { Intact = 0, Cracked = 1, Broken = 2 };

struct ServerStatus {
    uint8_t player_id;
    uint16_t x;
    uint16_t y;
    Direction direction;

    ServerStatus(uint8_t player_id, uint16_t x, uint16_t y, Direction direction):
            player_id(player_id), x(x), y(y), direction(direction) {}
};

struct ServerWelcome {
    uint8_t player_id;
};

struct ServerLeave {
    uint8_t player_id;
};

struct ServerResource {
    uint16_t x;
    uint16_t y;
    std::string content_id;
    ResourceDamage damage;
};

struct ServerDropped {
    uint16_t x;
    uint16_t y;
    std::string content_id;
};

// count 0 = slot vacío (content_id vacío).
struct ServerInventorySlot {
    uint8_t slot;
    uint8_t count;
    std::string content_id;
};

// El jugador local obtuvo un contenido (evento puntual, no estado).
struct ServerObtained {
    std::string content_id;
};

// El jugador player_id usó una herramienta (animación de golpe).
struct ServerToolUsed {
    uint8_t player_id;
};

// Impacto confirmado: no depende de cambios del estado visual.
struct ServerResourceHit {
    uint16_t x;
    uint16_t y;
    bool destroyed;
};

// Borra el estado ya aplicado (mapa, inventario). El lote que sigue es
// siempre el estado completo, así que lo que no vuelva ya no existe.
struct ServerReset {};

using ServerMessage =
        std::variant<ServerStatus, ServerLeave, ServerResource, ServerDropped, ServerInventorySlot,
                     ServerObtained, ServerToolUsed, ServerResourceHit, ServerReset>;

#endif
