#include "world.h"

#include <stdexcept>
#include <utility>

World::World(uint16_t w, uint16_t h): width(w), height(h) {
    this->grid.resize(this->height);
    for (uint16_t y = 0; y < this->height; ++y) {
        this->grid[y].resize(this->width);
    }
}

// ---------------------------------------------------------
// COLISIONES
// ---------------------------------------------------------
bool World::can_walk_to(uint16_t x, uint16_t y) const {
    // 1. ¿Se sale de los bordes del mapa?
    if (x >= this->width || y >= this->height) {
        return false;
    }

    const Cell& target_cell = this->grid[y][x];

    // 2. ¿Es una pared estática del escenario?
    if (target_cell.is_wall) {
        return false;
    }

    // 3. ¿Hay una entidad sólida en esa celda? ¿Se puede caminar encima?
    if (!target_cell.is_empty() && !target_cell.entity->is_walkable()) {
        return false;
    }

    // 4. ¿Hay otro jugador parado ahí?
    for (const auto& [id, player]: this->players) {
        if (player->get_x() == x && player->get_y() == y) {
            return false;
        }
    }

    // Si pasamos todos los filtros, la celda libre.
    return true;
}

// ---------------------------------------------------------
// MOVIMIENTO DEL JUGADOR
// ---------------------------------------------------------
void World::move_player(uint8_t player_id, Direction dir) {
    Player& p = *this->players.at(player_id);
    
    p.set_facing(dir);

    // Calculamos qué coordenada tiene enfrente ahora mismo
    auto [next_x, next_y] = p.get_target_tile();

    // Chequeamos colisiones
    if (this->can_walk_to(next_x, next_y)) {
        // Finalmente, movemos fsicamente al jugador
        p.set_position(next_x, next_y);
        Cell& current_cell = this->grid[next_y][next_x];
        
        if (!current_cell.is_empty()) {
            current_cell.entity->interact(p, ToolType::HAND, *this);
        }
    }
}

const Player& World::get_player(uint8_t player_id) const { return *this->players.at(player_id); }

// ---------------------------------------------------------
// GESTIÓN DE ENTIDADES en O(1)
// ---------------------------------------------------------
void World::set_entity_at(uint16_t x, uint16_t y, std::unique_ptr<IEntity> new_entity) {
    // Simpre chequeamos que no salga del maa.
    if (x >= this->width || y >= this->height)
        return;

    // TODO: (Opción A - Manejo de múltiples entidades por celda)
    // Actualmente, reasignar `grid[y][x].entity` destruye la entidad previa por RAII.
    // Esto funciona para el flujo básico (SilverRock -> DroppedItem).
    // Si en el futuro se spawnea un drop sobre una celda ya ocupada (ej. varios huevos o drops
    // simultáneos):
    // 1. Verificar si !grid[y][x].is_empty().
    // 2. Si está ocupada, buscar la celda adyacente desocupada más cercana (UP, DOWN, LEFT, RIGHT).
    // 3. Colocar el nuevo `new_entity` allí para evitar sobreescribir y perder el drop existente.

    new_entity->set_position(x, y);

    this->grid[y][x].entity = std::move(new_entity);
}

void World::remove_entity_at(uint16_t x, uint16_t y) {
    if (x >= this->width || y >= this->height)
        return;

    this->grid[y][x].entity.reset();
}

void World::add_player(uint8_t player_id, std::unique_ptr<Player> player) {
    this->players[player_id] = std::move(player);
}

void World::use_tool(uint8_t player_id) {
    Player& p = *this->players.at(player_id);
    auto [tx, ty] = p.get_target_tile(); 

    if (tx >= this->width || ty >= this->height) return;

    Cell& target_cell = this->grid[ty][tx];
    
    if (!target_cell.is_empty()) {
        target_cell.entity->interact(p, p.get_equipped_tool(), *this);
    }
}
void World::remove_player(uint8_t player_id) { this->players.erase(player_id); }
