#ifndef COMMON_SERVER_STATUS_H
#define COMMON_SERVER_STATUS_H

#include <cstdint>
#include <string>
#include <variant>
#include <vector>

#include "common/model/draft/CharacterAppearance.h"
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
    SyncEnd = 18,
    Crop = 19,
    GroundEffect = 20,
    Enemy = 21,
    Projectile = 22,
    Chat = 23,
    GiftRequest = 24,
    GiftAnswered = 25,
    SetCompleted = 26,
    Victory = 27,
    Gold = 28,
    NpcPlacement = 29,
    MarketOffer = 30,
    MapChange = 31,
    PlayerHeld = 32,
    GiftAccepted = 33,
    PlayerName = 34,
    InventoryFull = 35,
};

// Estado de daño visible de un recurso picable.
enum class ResourceDamage : uint8_t { Intact = 0, Cracked = 1, Broken = 2 };

// Consecuencia visible en el suelo de una celda (tierra arada / regada /
// charco de sangre).
enum class GroundEffect : uint8_t { Dug = 0, Watered = 1, Blood = 2 };

struct ServerStatus {
    uint8_t player_id;
    uint16_t x;
    uint16_t y;
    Direction direction;
    // Personalizacion del personaje: indices de capa sin validar. El server
    // no conoce los counts de characters.toml, asi que el cliente los acota
    // al dibujar y cae al 0 (default) si estan fuera de rango. Viaja en cada
    // lote para que un joiner tarde la reciba de todos los jugadores.
    CharacterAppearance appearance;
    // Vida y energia del jugador (energia escalada x100).
    uint16_t health;
    uint16_t max_health;
    uint16_t energy;      // x100
    uint16_t max_energy;  // x100
    // 1 = desmayado por energía o por fin de horario. 0 = no.
    uint8_t fainted;
    ServerStatus(uint8_t player_id, uint16_t x, uint16_t y, Direction direction,
                 const CharacterAppearance& appearance,
                 uint16_t health, uint16_t max_health, uint16_t energy, uint16_t max_energy,
                 uint8_t fainted):
            player_id(player_id), x(x), y(y), direction(direction), appearance(appearance),
            health(health), max_health(max_health), energy(energy), max_energy(max_energy),
            fainted(fainted) {}
};

struct ServerWelcome {
    uint8_t player_id;
    std::string worldName;
    ServerWelcome(uint8_t player_id, const std::string& worldName = {}):
            player_id(player_id), worldName(worldName) {}
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

// count 0 = slot vacío (content_id vacío, tradable y quality indiferentes).
// tradable: lo decide el servidor (Item::can_be_traded): las herramientas
// no se regalan y el cliente ni ofrece el intercambio. Es la fuente única
// de esa regla, así no se duplica el vocabulario en el cliente.
// quality: 1..5 = BASE..PERFECT (el mismo orden que ServerCrop y que
// Quality del servidor). Vale 1 (BASE) para herramientas y cualquier ítem
// sin calidad propia.
struct ServerInventorySlot {
    uint8_t slot;
    uint8_t count;
    std::string content_id;
    bool tradable;
    uint8_t quality = 1;
};

// El jugador local obtuvo un contenido (evento puntual, no estado).
struct ServerObtained {
    std::string content_id;
};

// El jugador player_id usó una herramienta (animación de golpe). content_id es
// el contenido equipado (vacío = manos); el cliente elige la animación por él.
struct ServerToolUsed {
    uint8_t player_id;
    std::string content_id;
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

// Último mensaje de cada lote de estado: sin él el cliente no sabe dónde
// termina el lote y puede dibujar un frame entre el Reset y las entidades
// (mundo vacío = el parpadeo). El cliente aplica el lote recién acá.
struct ServerSyncEnd {};

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

// Enemigo del mundo (esqueleto, orco, vampiro, skull). kind es el vocabulario de
// enemies.toml; health 0 = muerto (el cliente muestra la pose de muerte).
struct ServerEnemy {
    uint16_t id;
    std::string kind;
    uint16_t x;
    uint16_t y;
    Direction direction;
    uint8_t health;
    EnemyAction action;
};

// Llama de un skull: viaja en línea recta por celdas. kind es el vocabulario de
// enemies.toml (skull_fire, skull_fire_green); no lleva vida ni acción.
struct ServerProjectile {
    uint16_t id;
    std::string kind;
    uint16_t x;
    uint16_t y;
    Direction direction;
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

// Cultivo plantado. crop_id es el contentId del producto (ej. "strawberry").
// stage: 0 semilla, 1..5 crecimiento, 6 cosecha. quality: 1..5 = BASE..PERFECT,
// el mismo orden que qualities de crops.toml (solo importa en la cosecha).
struct ServerCrop {
    uint16_t x;
    uint16_t y;
    std::string crop_id;
    uint8_t stage;
    bool needs_water;
    bool interactive;
    uint8_t quality;
};

// Consecuencia en una celda: tierra arada/regada, o charco de sangre
// (Blood, estado PERSISTENTE del dia: se seca al terminar el dia o lo
// consume una vampira). Va a todos los jugadores para que vean de donde
// pueden revivir los vampiros. Una celda regada manda Dug y despues Watered.
struct ServerGroundEffect {
    uint16_t x;
    uint16_t y;
    GroundEffect effect;
};

// Mensaje de chat: player_id y characterName identifican al emisor (0/vacío si
// es un mensaje de sistema, ej. respuesta de un cheat o error). system = true
// indica que el cliente debe mostrarlo sin prefijo de jugador ("Servidor: ...").
struct ServerChat {
    uint8_t player_id;
    std::string characterName;
    std::string text;
    bool system;
};

// Pedido de regalo hacia el receptor: el emisor ofreció su ítem equipado
// (content_id) y el server lo reenvía junto con su characterName (el mismo
// que ServerChat) para el copy del popup del receptor.
struct ServerGiftRequest {
    uint8_t sender_id;
    std::string characterName;
    std::string content_id;
};

// El receptor contestó el pedido de regalo (aceptando o rechazando): el
// server lo manda SOLO al emisor, que así cierra su popup de espera.
// Evento sin payload, igual que ServerDayEnd.
struct ServerGiftAnswered {};

struct ServerSetCompleted {
    uint8_t set_id;
};

struct ServerVictory {};

struct ServerGold {
    uint16_t gold;
};

// Posición física de los pies del mercader.
struct ServerNpcPlacement {
    std::string id;
    uint16_t x;
    uint16_t y;
};

// Id del mapa actual del jugador: el cliente cambia de escena por él.
struct ServerMapChange {
    std::string mapId;
};

enum class ServerMarketSide : uint8_t { Sells = 0, Buys = 1 };

struct ServerMarketEntry {
    std::string content_id;
    std::string label;
    ServerMarketSide side;
    bool has_price;
    uint16_t price;
};

struct ServerMarketOffer {
    std::string merchant_id;
    std::vector<ServerMarketEntry> entries;
};

// Estado público del objeto seleccionado; content_id vacío = manos libres.
struct ServerPlayerHeld {
    uint8_t player_id;
    std::string content_id;
};

// Evento emitido solo después de transferir el regalo correctamente.
struct ServerGiftAccepted {
    uint8_t receiver_id;
};

struct ServerPlayerName {
    uint8_t player_id;
    std::string characterName;
};

struct ServerInventoryFull {};

using ServerMessage =
        std::variant<ServerStatus, ServerLeave, ServerResource, ServerDropped, ServerInventorySlot,
                     ServerObtained, ServerToolUsed, ServerResourceHit, ServerReset, ServerAnimal,
                     ServerSleep, ServerFurniture, ServerDayEnd, ServerDay, ServerSleepingPlayers,
                     ServerClock, ServerSyncEnd, ServerCrop, ServerGroundEffect, ServerEnemy, ServerProjectile,
                     ServerChat, ServerGiftRequest, ServerGiftAnswered, ServerSetCompleted, ServerVictory,
                     ServerGold, ServerNpcPlacement, ServerMarketOffer, ServerMapChange,
                     ServerPlayerHeld, ServerGiftAccepted, ServerPlayerName, ServerInventoryFull>;

#endif
