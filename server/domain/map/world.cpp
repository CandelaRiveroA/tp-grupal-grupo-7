#include "world.h"
#include "../crops/crop.h"
#include "../crops/strawberry.h"
#include "../crops/green_onion.h"
#include "../crops/potato.h"
#include "../crops/pumpkin.h"
#include "../inventory/item_types.h"

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
    if (x >= this->width || y >= this->height) {
        return false;
    }

    const Cell& target_cell = this->grid[y][x];

    if (target_cell.is_wall) {
        return false;
    }

    for (const auto& entity : target_cell.entities) {
        if (!entity->is_walkable()) {
            return false;
        }
    }

    for (const auto& [id, player]: this->players) {
        if (player->get_x() == x && player->get_y() == y) {
            return false;
        }
    }

    return true;
}

// ---------------------------------------------------------
// MOVIMIENTO DEL JUGADOR
// ---------------------------------------------------------
void World::move_player(uint8_t player_id, Direction dir) {
    Player& p = *this->players.at(player_id);
    p.set_facing(dir);

    auto [next_x, next_y] = p.get_target_tile();

    // Chequeamos colisiones
    if (this->can_walk_to(next_x, next_y)) {
        p.set_position(next_x, next_y);
        Cell& current_cell = this->grid[next_y][next_x];

        // Efecto imán / Auto-Pickup: Interactuamos con TODAS las entidades pisables (ej. varios drops)
        if (!current_cell.is_empty()) {
            // Usamos un vector temporal de punteros para iterar de forma segura, 
            // ya que `interact` podría eliminar la entidad de la lista durante el ciclo.
            std::vector<IEntity*> iteratable_entities;
            for (const auto& ent : current_cell.entities) {
                iteratable_entities.push_back(ent.get());
            }
            
            for (IEntity* ent : iteratable_entities) {
                ent->interact(p, ToolType::HAND, *this);
            }
        }
    }
}

const Player& World::get_player(uint8_t player_id) const { return *this->players.at(player_id); }

std::vector<uint8_t> World::get_player_ids() const {
    std::vector<uint8_t> ids;
    ids.reserve(this->players.size());
    for (const auto& [id, player]: this->players) {
        ids.push_back(id);
    }
    return ids;
}

uint16_t World::get_width() const { return this->width; }

uint16_t World::get_height() const { return this->height; }

const std::vector<IEntity*> World::get_entity_at(uint16_t x, uint16_t y) const {
    std::vector<IEntity*> found_entities;
    
    if (x >= this->width || y >= this->height) {
        return found_entities;
    }

    const Cell& target_cell = this->grid[y][x];
    
    for (const auto& ent : target_cell.entities) {
        found_entities.push_back(ent.get());
    }

    return found_entities;
}

// ---------------------------------------------------------
// GESTIÓN DE ENTIDADES en O(1)
// ---------------------------------------------------------
void World::set_entity_at(uint16_t x, uint16_t y, std::unique_ptr<IEntity> new_entity) {
    if (x >= this->width || y >= this->height)
        return;

    new_entity->set_position(x, y);
    this->grid[y][x].entities.push_back(std::move(new_entity));
}

void World::remove_entity(IEntity* target_entity) {
    if (!target_entity) return;

    uint16_t x = target_entity->get_x();
    uint16_t y = target_entity->get_y();

    if (x >= this->width || y >= this->height) return;

    auto& cell_entities = this->grid[y][x].entities;
    
    // Buscamos y borramos específicamente la entidad solicitada
    for (auto it = cell_entities.begin(); it != cell_entities.end(); ++it) {
        if (it->get() == target_entity) {
            cell_entities.erase(it); // Se destruye automáticamente
            break;
        }
    }
}

void World::add_player(uint8_t player_id, std::unique_ptr<Player> player) {
    this->players[player_id] = std::move(player);
}

void World::use_tool(uint8_t player_id) {
    Player& p = *this->players.at(player_id);
    auto [tx, ty] = p.get_target_tile();

    if (tx >= this->width || ty >= this->height)
        return;

    Cell& target_cell = this->grid[ty][tx];
    
    ToolType tool = p.get_equipped_tool();

    if (tool == ToolType::SHOVEL) {
        target_cell.till_soil(); 
    } 
    else if (tool == ToolType::WATERING_CAN) {
        target_cell.water_soil();
    }

    // Interactuar de frente (picar, levantar manual)
    if (!target_cell.is_empty()) {
        std::vector<IEntity*> iteratable_entities;
        for (const auto& ent : target_cell.entities) {
            iteratable_entities.push_back(ent.get());
        }
        
        for (IEntity* ent : iteratable_entities) {
            ent->interact(p, p.get_equipped_tool(), *this);
        }
    }
}

void World::try_plant_seed(uint8_t player_id) {
    // TODO: Se parece mucho a use_tool, refactorizar para no repetir código.
    Player& p = *this->players.at(player_id);
    auto [tx, ty] = p.get_target_tile(); 

    if (tx >= this->width || ty >= this->height) return;

    Cell& target_cell = this->grid[ty][tx];

    if (!target_cell.is_soil_tilled() && !target_cell.is_soil_watered()) return;
    if (!target_cell.is_empty()) return;

    const Item* active_item = p.peek_equipped_item();
    if (!active_item) return; 
    
    // Si el jugador equipó una herramienta real (Pico, Regadera, Pala), abortamos.
    // Solo permitimos semillas, las cuales mapéan a HAND en tu player[cite: 8].
    if (p.get_equipped_tool() != ToolType::HAND) return;

    ItemID id = active_item->get_id();
    std::unique_ptr<Crop> new_crop = nullptr;

    //TODO: Sacar este switch a un factory method en CropFactory para no tener que tocar World.cpp cada vez que agregamos un nuevo cultivo.

    switch (id) {
        case ItemID::STRAWBERRY_SEED:
            new_crop = std::make_unique<Strawberry>(tx, ty);
            break;
        case ItemID::ONION_SEED:
            new_crop = std::make_unique<GreenOnion>(tx, ty);
            break;
        case ItemID::POTATO_SEED:
            new_crop = std::make_unique<Potato>(tx, ty);
            break;
        case ItemID::PUMPKIN_SEED:
            new_crop = std::make_unique<Pumpkin>(tx, ty);
            break;
        default:
            return; 
    }

    if (new_crop) {
        p.consume_equipped_item(); 
        target_cell.entities.push_back(std::move(new_crop)); 
    }
}

void World::record_resource_hit(uint16_t x, uint16_t y, bool destroyed) {
    resource_hits.push_back(ResourceHit{x, y, destroyed});
}

std::vector<ResourceHit> World::take_resource_hits() {
    std::vector<ResourceHit> hits;
    hits.swap(resource_hits);
    return hits;
}

void World::set_player_facing(uint8_t player_id, Direction dir) {
    this->players.at(player_id)->set_facing(dir);
}

void World::take_item(uint8_t player_id) {
    Player& p = *this->players.at(player_id);
    Cell& own_cell = this->grid[p.get_y()][p.get_x()];

    if (!own_cell.is_empty()) {
        std::vector<IEntity*> iteratable_entities;
        for (const auto& ent : own_cell.entities) {
            iteratable_entities.push_back(ent.get());
        }
        
        for (IEntity* ent : iteratable_entities) {
            ent->interact(p, ToolType::HAND, *this);
        }
    }
}

void World::remove_player(uint8_t player_id) { this->players.erase(player_id); }

void World::dry_soil() {
    for (uint16_t y = 0; y < this->height; ++y) {
        for (uint16_t x = 0; x < this->width; ++x) {
            if (this->grid[y][x].is_soil_watered()) {
                this->grid[y][x].dry_soil();
            }
        }
    }
}

void World::process_end_of_day_floor() {
    for (uint16_t y = 0; y < this->height; ++y) {
        for (uint16_t x = 0; x < this->width; ++x) {
            Cell& current_cell = this->grid[y][x];

            // 1. Notificamos a las entidades (Los cultivos leen el suelo y crecen)
            for (auto& entity_ptr : current_cell.entities) {
                 entity_ptr->update(*this); 
            }

            // 2. Comprobamos si hay algún cultivo plantado
            // TODO: Hace con dynamic_cast para asegurarse de que es un Crop y no un DroppedItem, pero para el modelo esto sirve.
            bool has_crop = !current_cell.is_empty(); 

            // 3. Aplicamos las reglas de estado del suelo
            if (current_cell.is_soil_watered()) {
                // Regla 1: Todo piso regado se seca (pasa a TILLED), tenga o no cultivo
                current_cell.dry_soil(); 
            } 
            else if (current_cell.is_soil_tilled() && !has_crop) {
                // Regla 2: Arado + Sin Cultivo = Vuelve a NORMAL
                current_cell.reset_soil();
            }
            // Regla implícita: Arado + Con Cultivo + No regado = Queda igual (TILLED).
        }
    }
}

Cell& World::get_cell(uint16_t x, uint16_t y) {
    if (x >= this->width || y >= this->height) {
        throw std::out_of_range("Cell coordinates out of bounds");
    }
    return this->grid[y][x];
}

