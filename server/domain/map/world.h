#ifndef WORLD_H
#define WORLD_H

#include <memory>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "../entities/player.h"

#include "cell.h"
#include "player_pickup.h"
#include "resource_hit.h"

class Animal;

class World {
private:
    uint16_t width;
    uint16_t height;

    std::vector<std::vector<Cell>> grid;
    // No poseen: los dueños son las celdas. Se mantienen para tick_day y ordeñe.
    std::vector<Animal*> animals;
    std::vector<ResourceHit> resource_hits;
    std::vector<PlayerPickup> pickups;

    // Sueño: jugadores en la cama (sleepers) y los que ya dieron OK
    // (accepted). accepted ⊆ sleepers, así que vaciar sleepers vacía
    // accepted. Quien está en sleepers no se puede mover.
    std::unordered_set<uint8_t> sleepers;
    std::unordered_set<uint8_t> accepted;
    // Día terminado y esperando que todos despierten (woke ⊆ players). Mientras
    // tanto el reloj y los animales están quietos.
    bool waking;
    std::unordered_set<uint8_t> woke;
    // Transiciones del día, consumidas por Match (una sola vez cada una).
    bool day_ended;
    bool day_started;
    // Día de juego: de 1 a 28, después del 28 vuelve al 1.
    uint8_t day;
    // TODO: Persistencia del inventario del jugador.

    // Diccionario de jugadores activos en este mapa (por ID de cliente/jugador)
    std::unordered_map<uint8_t, std::unique_ptr<Player>> players;

public:
    World(uint16_t w, uint16_t h);
    ~World() = default;
    World(const World&) = delete;
    World& operator=(const World&) = delete;

    // --- Detección de Colisiones en O(1) ---
    bool can_walk_to(uint16_t x, uint16_t y) const;

    // --- Gestión de Jugadores ---
    void add_player(uint8_t player_id, std::unique_ptr<Player> player);
    void remove_player(uint8_t player_id);
    void move_player(uint8_t player_id, Direction dir);
    const Player& get_player(uint8_t player_id) const;
    std::vector<uint8_t> get_player_ids() const;

    uint16_t get_width() const;
    uint16_t get_height() const;
    const std::vector<IEntity*> get_entity_at(uint16_t x, uint16_t y) const;

    // --- Gestión de Entidades Físicas ---
    void set_entity_at(uint16_t x, uint16_t y, std::unique_ptr<IEntity> new_entity);
    void remove_entity(IEntity* target_entity);
    void remove_entity_at(uint16_t x, uint16_t y);

    void add_animal(std::unique_ptr<Animal> animal);

    // Avanza un día: cada animal reacciona a su update (huevos, leche lista).
    void tick_day();
    // Pasa al día siguiente (contador de día) y corre tick_day. Lo llaman el
    // timeout y el sueño.
    void advance_day();
    uint8_t get_day() const;

    // Un paso del reloj de animales: cada uno se mueve en su dirección actual
    // o elige una nueva si expiró el tiempo.
    void tick_animals();

    // Mueve una entidad a otra celda; no hace nada si el destino está bloqueado.
    void move_entity(IEntity* entity, uint16_t x, uint16_t y);

    // ---
    void use_tool(uint8_t player_id);
    void record_resource_hit(uint16_t x, uint16_t y, bool destroyed);
    std::vector<ResourceHit> take_resource_hits();
    void record_pickup(uint8_t player_id, ItemID item);
    std::vector<PlayerPickup> take_pickups();
    void set_player_facing(uint8_t player_id, Direction dir);
    // Personalizacion del personaje del jugador (indices de capa, sin validar).
    void set_player_appearance(uint8_t player_id, const CharacterAppearance& appearance);
    void take_item(uint8_t player_id);

    // --- Sueño (votación para dormir y pasar de día) ---
    // El jugador pisa la cama: entra en la votación y queda sin movimiento.
    void record_sleep_request(uint8_t player_id);
    // Responde al aviso: accept = confirma dormir, false = cancelar (sale
    // de la cama, le devuelve el movimiento y borra solo su OK).
    void answer_sleep(uint8_t player_id, bool accept);
    uint8_t sleeping_count() const;
    bool is_sleeping(uint8_t player_id) const;
    // Cancela la votación abierta sin mover a nadie.
    void clear_sleep();
    void set_player_position(uint8_t player_id, uint16_t x, uint16_t y);

    // --- Fin de día: todos tienen que despertar (OK) antes de arrancar el siguiente ---
    // Termina el día (timeout o sueño) y congela el reloj hasta despertar.
    void end_day();
    // El jugador apretó OK: cuando despiertan todos, el día avanza.
    void answer_wake(uint8_t player_id);
    bool is_waking() const;
    // true una sola vez por transición. Los consume Match.
    bool take_day_ended();
    bool take_day_started();

private:
    void try_advance_sleep();
    void try_start_day();
};

#endif  // WORLD_H
