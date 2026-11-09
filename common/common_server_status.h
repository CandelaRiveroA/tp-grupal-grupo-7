#ifndef COMMON_SERVER_STATUS_H
#define COMMON_SERVER_STATUS_H

#include <cstdint>
#include <string>
#include <variant>
#include <vector>

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
    Animal = 11,
    Sleep = 12,
    Furniture = 13,
    Day = 14,
    SleepingPlayers = 15,
    Clock = 16,
    DayEnd = 17,
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

// true = se levanta con Take y anima (huevo); false = solo suma al inventario.
struct ServerDropped {
    uint16_t x;
    uint16_t y;
    std::string content_id;
    bool interactive;
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

// Animal del mundo (vaca, gallina). kind es el vocabulario de characters.toml;
// ready_to_milk avisa al jugador que la vaca se puede ordeñar; direction es hacia dónde mira.
struct ServerAnimal {
    uint8_t id;
    std::string kind;
    uint16_t x;
    uint16_t y;
    Direction direction;
    bool ready_to_milk;
};

// Aviso de sueño: sleeping de total jugadores están en la cama. Mientras
// sleeping > 0 el aviso queda abierto; en 0 se cierra (nadie duerme más).
struct ServerSleep {
    uint8_t sleeping;
    uint8_t total;
};

// Se terminó el día (sueño o timeout): la pantalla queda en negro hasta que
// todos manden WakeMessage. Sin payload.
struct ServerDayEnd {};

// Amaneció: todos despertaron y arranca el día nuevo. Sin payload.
struct ServerDay {};

// Quiénes están en la cama (sus IDs). Va a todos: el aviso ServerSleep solo
// le dice al durmiente cuántos son, no quién duerme.
struct ServerSleepingPlayers {
    std::vector<uint8_t> players;
};

// Día y hora de juego: día de 1 a 28, y minutos desde las 6:00 (0 a 1200,
// las 2:00 del día siguiente). La hora cambia de a 10 minutos de juego.
struct ServerClock {
    uint8_t day;
    uint16_t minutes;
};

// Mueble colocado en el mundo (cama, mesa, silla). Va aparte de
// ServerDropped para que el cliente no lo trate como un ítem del piso.
struct ServerFurniture {
    uint16_t x;
    uint16_t y;
    std::string content_id;
};

using ServerMessage =
        std::variant<ServerStatus, ServerLeave, ServerResource, ServerDropped, ServerInventorySlot,
                     ServerObtained, ServerToolUsed, ServerResourceHit, ServerReset, ServerAnimal,
                     ServerSleep, ServerFurniture, ServerDayEnd, ServerDay, ServerSleepingPlayers,
                     ServerClock>;

#endif
