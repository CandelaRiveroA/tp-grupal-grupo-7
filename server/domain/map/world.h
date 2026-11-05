#ifndef WORLD_H
#define WORLD_H

#include <memory>
#include <unordered_map>
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
    void take_item(uint8_t player_id);
};

#endif  // WORLD_H
