#include "world.h"
#include "../crops/crop.h"
#include "../crops/strawberry.h"
#include "../crops/green_onion.h"
#include "../crops/potato.h"
#include "../crops/pumpkin.h"
#include "../inventory/item_types.h"

#include <cstdlib>
#include <stdexcept>
#include <utility>

#include "../animals/animal.h"

// ciclo fijo de 28 días que vuelve al día 1.
#define kDaysPerCycle 28

World::World(uint16_t w, uint16_t h):
        width(w), height(h), waking(false), day_ended(false), day_started(false), day(1) {
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

    // cppcheck-suppress useStlAlgorithm; el loop corta al primer bloqueo y también lee is_wall.
    for (const auto& entity: target_cell.entities) {
        if (!entity->is_walkable()) {
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

    // Durmiendo no se mueve: solo cancelar le devuelve el movimiento.
    if (this->sleepers.count(player_id) > 0) {
        return;
    }

    p.set_facing(dir);

    auto [next_x, next_y] = p.get_target_tile();

    // Chequeamos colisiones
    if (this->can_walk_to(next_x, next_y)) {
        p.set_position(next_x, next_y);
        Cell& current_cell = this->grid[next_y][next_x];

        // Efecto imán / Auto-Pickup: Interactuamos con TODAS las entidades pisables (ej. varios
        // drops)
        if (!current_cell.is_empty()) {
            // Usamos un vector temporal de punteros para iterar de forma segura,
            // ya que `interact` podría eliminar la entidad de la lista durante el ciclo.
            std::vector<IEntity*> iteratable_entities;
            for (const auto& ent: current_cell.entities) {
                // cppcheck-suppress useStlAlgorithm; copia cruda para poder borrar durante
                // interact.
                iteratable_entities.push_back(ent.get());
            }

            for (IEntity* ent: iteratable_entities) {
                // El huevo espera el Take (para poder animarlo): el imán lo saltea.
                if (ent->pickup_on_take()) {
                    continue;
                }
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

    for (const auto& ent: target_cell.entities) {
        // cppcheck-suppress useStlAlgorithm; la salida es vector<IEntity*>, no una copia de la
        // celda.
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
    if (!target_entity)
        return;

    uint16_t x = target_entity->get_x();
    uint16_t y = target_entity->get_y();

    if (x >= this->width || y >= this->height)
        return;

    auto& cell_entities = this->grid[y][x].entities;

    // Buscamos y borramos específicamente la entidad solicitada
    for (auto it = cell_entities.begin(); it != cell_entities.end(); ++it) {
        if (it->get() == target_entity) {
            cell_entities.erase(it);  // Se destruye automáticamente
            break;
        }
    }
}

void World::remove_entity_at(uint16_t x, uint16_t y) {
    if (x >= this->width || y >= this->height)
        return;

    auto& cell_entities = this->grid[y][x].entities;
    if (!cell_entities.empty()) {
        cell_entities.erase(cell_entities.begin());
    }
}

void World::add_animal(std::unique_ptr<Animal> animal) {
    const uint16_t x = animal->get_x();
    const uint16_t y = animal->get_y();
    set_entity_at(x, y, std::move(animal));
    // set_entity_at agrega al final de la celda: el puntero es el de la celda, no el del parámetro.
    animals.push_back(static_cast<Animal*>(this->grid[y][x].entities.back().get()));
}

void World::tick_day() {
    // Se copian los punteros: update puede poner huevos y cambiar las listas de celdas.
    const std::vector<Animal*> today = animals;
    for (Animal* animal: today) {
        animal->update(*this);
    }
}

void World::advance_day() {
    this->day = static_cast<uint8_t>(this->day % kDaysPerCycle + 1);
    this->tick_day();
}

uint8_t World::get_day() const { return this->day; }

void World::tick_animals() {
    for (Animal* animal: animals) {
        animal->wander(*this);
    }
}

void World::move_entity(IEntity* entity, uint16_t x, uint16_t y) {
    if (!entity || (x == entity->get_x() && y == entity->get_y())) {
        return;
    }
    if (!this->can_walk_to(x, y)) {
        return;
    }

    const uint16_t from_x = entity->get_x();
    const uint16_t from_y = entity->get_y();
    if (from_x >= this->width || from_y >= this->height) {
        return;
    }

    // El dueño de la entidad es la celda: hay que sacarlo de una y meterlo en la otra.
    std::unique_ptr<IEntity> moved;
    auto& from_entities = this->grid[from_y][from_x].entities;
    for (auto it = from_entities.begin(); it != from_entities.end(); ++it) {
        if (it->get() == entity) {
            moved = std::move(*it);
            from_entities.erase(it);
            break;
        }
    }
    if (!moved) {
        return;
    }

    entity->set_position(x, y);
    this->grid[y][x].entities.push_back(std::move(moved));
}

void World::add_player(uint8_t player_id, std::unique_ptr<Player> player) {
    this->players[player_id] = std::move(player);
}

void World::use_tool(uint8_t player_id) {
    // Durmiendo no se usan herramientas.
    if (this->sleepers.count(player_id) > 0) {
        return;
    }
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
        for (const auto& ent: target_cell.entities) {
            // cppcheck-suppress useStlAlgorithm; copia cruda para poder borrar durante interact.
            iteratable_entities.push_back(ent.get());
        }

        for (IEntity* ent: iteratable_entities) {
            ent->interact(p, p.get_equipped_tool(), *this);
        }
    }
}

void World::try_plant_seed(uint8_t player_id) {
    Player& p = *this->players.at(player_id);
    auto [tx, ty] = p.get_target_tile(); 

    if (tx >= this->width || ty >= this->height) return;

    Cell& target_cell = this->grid[ty][tx];

    if (!target_cell.is_soil_tilled() && !target_cell.is_soil_watered()) return;
    if (!target_cell.is_empty()) return;
    if (!target_cell.can_plant_seed()) return;

    const Item* active_item = p.peek_equipped_item();
    if (!active_item) return; 
    
    if (p.get_equipped_tool() != ToolType::HAND) return;

    ItemID id = active_item->get_id();
    std::unique_ptr<Crop> new_crop = nullptr;

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
        // Usamos el setter del mundo para guardarlo correctamente en la grilla
        this->set_entity_at(tx, ty, std::move(new_crop)); 
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

void World::record_pickup(uint8_t player_id, ItemID item) {
    pickups.push_back(PlayerPickup{player_id, item});
}

std::vector<PlayerPickup> World::take_pickups() {
    std::vector<PlayerPickup> taken;
    taken.swap(pickups);
    return taken;
}

void World::set_player_facing(uint8_t player_id, Direction dir) {
    this->players.at(player_id)->set_facing(dir);
}

void World::set_player_appearance(uint8_t player_id, const CharacterAppearance& appearance) {
    this->players.at(player_id)->set_appearance(appearance);
}

void World::take_item(uint8_t player_id) {
    // Durmiendo no se toma nada ni se ordeña.
    if (this->sleepers.count(player_id) > 0) {
        return;
    }
    Player& p = *this->players.at(player_id);

    // Ordeñar: un animal que responde a la mano a una celda de distancia (las 8
    // vecinas) tiene prioridad sobre lo que hay en el piso.
    for (Animal* animal: animals) {
        const int dx = animal->get_x() - p.get_x();
        const int dy = animal->get_y() - p.get_y();
        if (std::abs(dx) <= 1 && std::abs(dy) <= 1 && animal->try_hand_action(p, *this)) {
            return;
        }
    }

    // Huevos: se levantan desde las 8 vecinas, como la vaca (el cliente anima el primero).
    for (int dy = -1; dy <= 1; ++dy) {
        for (int dx = -1; dx <= 1; ++dx) {
            // Coordenadas negativas se vuelven enormes y get_entity_at las descarta.
            const auto x = static_cast<uint16_t>(p.get_x() + dx);
            const auto y = static_cast<uint16_t>(p.get_y() + dy);

            for (IEntity* ent: get_entity_at(x, y)) {
                if (ent->pickup_on_take()) {
                    ent->interact(p, ToolType::HAND, *this);
                    return;
                }
            }
        }
    }

    Cell& own_cell = this->grid[p.get_y()][p.get_x()];

    if (!own_cell.is_empty()) {
        std::vector<IEntity*> iteratable_entities;
        for (const auto& ent: own_cell.entities) {
            // cppcheck-suppress useStlAlgorithm; copia cruda para poder borrar durante interact.
            iteratable_entities.push_back(ent.get());
        }

        for (IEntity* ent: iteratable_entities) {
            ent->interact(p, ToolType::HAND, *this);
        }
    }
}

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

void World::remove_player(uint8_t player_id) {
    this->players.erase(player_id);
    // Nadie queda esperando a un jugador que ya no está en la partida.
    this->sleepers.erase(player_id);
    this->accepted.erase(player_id);
    this->woke.erase(player_id);
    // Si el que se fue era el único que faltaba aceptar o despertar, el día tiene que avanzar.
    this->try_advance_sleep();
    this->try_start_day();
}

void World::set_player_position(uint8_t player_id, uint16_t x, uint16_t y) {
    this->players.at(player_id)->set_position(x, y);
}


// ---------------------------------------------------------
// SUEÑO
// ---------------------------------------------------------
void World::clear_sleep() {
    this->sleepers.clear();
    this->accepted.clear();
}

void World::record_sleep_request(uint8_t player_id) {
    // Idempotente: pisar la cama o usar la mano encima vuelve a llamar acá.
    if (this->waking || this->sleepers.count(player_id) > 0) {
        return;
    }
    this->sleepers.insert(player_id);
}

void World::answer_sleep(uint8_t player_id, bool accept) {
    // Solo quien está en la cama responde al aviso.
    if (this->sleepers.count(player_id) == 0) {
        return;
    }

    if (!accept) {
        // Cancelar: sale de la cama y se borra solo su OK. Los OK de los demás
        // siguen valiendo (así nadie que ya votó queda esperando en vano).
        this->sleepers.erase(player_id);
        this->accepted.erase(player_id);
        return;
    }

    this->accepted.insert(player_id);
    this->try_advance_sleep();
}

// Termina el día cuando todos los de la partida están en la cama y aceptaron.
void World::try_advance_sleep() {
    if (this->players.empty() || this->sleepers.size() < this->players.size() ||
        this->accepted.size() < this->players.size()) {
        return;
    }
    this->end_day();
}

void World::end_day() {
    if (this->waking) {
        return;
    }
    this->clear_sleep();
    this->waking = true;
    this->woke.clear();
    this->day_ended = true;
}

void World::answer_wake(uint8_t player_id) {
    if (!this->waking || this->players.count(player_id) == 0) {
        return;
    }
    this->woke.insert(player_id);
    this->try_start_day();
}

// Arranca el día cuando despertaron todos los de la partida.
void World::try_start_day() {
    if (!this->waking || this->players.empty() || this->woke.size() < this->players.size()) {
        return;
    }
    this->waking = false;
    this->woke.clear();
    this->advance_day();
    this->day_started = true;
}

bool World::is_waking() const { return this->waking; }

bool World::take_day_ended() {
    const bool ended = this->day_ended;
    this->day_ended = false;
    return ended;
}

bool World::take_day_started() {
    const bool started = this->day_started;
    this->day_started = false;
    return started;
}

uint8_t World::sleeping_count() const { return static_cast<uint8_t>(this->sleepers.size()); }

bool World::is_sleeping(uint8_t player_id) const { return this->sleepers.count(player_id) > 0; }
