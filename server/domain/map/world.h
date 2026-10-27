#ifndef WORLD_H
#define WORLD_H

#include <memory>
#include <unordered_map>
#include <vector>

#include "../entities/player.h"

#include "cell.h"

class World {
private:
    uint16_t width;
    uint16_t height;

    std::vector<std::vector<Cell>> grid;

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
    // TODO: Implementar  disconnect_player para
    // cuando el Acceptor limpie al ClientHandler.
    void add_player(uint8_t player_id, std::unique_ptr<Player> player);
    void move_player(uint8_t player_id, Direction dir);
    const Player& get_player(uint8_t player_id) const;

    // --- Gestión de Entidades Físicas ---
    void set_entity_at(uint16_t x, uint16_t y, std::unique_ptr<IEntity> new_entity);
    void remove_entity_at(uint16_t x, uint16_t y);
};

#endif  // WORLD_H
