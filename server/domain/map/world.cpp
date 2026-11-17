#include "world.h"
#include "../crops/crop.h"
#include "../crops/strawberry.h"
#include "../crops/green_onion.h"
#include "../crops/potato.h"
#include "../crops/pumpkin.h"
#include "../inventory/item_types.h"

#include <algorithm>
#include <cstdlib>
#include <stdexcept>
#include <utility>

#include "../animals/animal.h"
#include "../enemies/enemy.h"
#include "../enemies/orc.h"
#include "../enemies/vampire.h"
#include "../enemies/vampiress.h"
#include "../entities/i_movable.h"

// ciclo fijo de 28 días que vuelve al día 1.
#define kDaysPerCycle 28
// Manada de orcos: tamaño entre kOrcPackMin y kOrcPackMax, celdas hasta kOrcPackRadius.
#define kOrcPackMin 3
#define kOrcPackMax 6
#define kOrcPackRadius 2
// Pareja de vampiros: celdas hasta kVampirePairRadius y al menos kVampirePairMinCells.
#define kVampirePairRadius 1
#define kVampirePairMinCells 2
// Pasos de EnemyTick (400 ms) entre la muerte y la cama: deja correr la
// animación de desmayo del cliente (30 ticks ≈ 500 ms) más el lote de
// red (≤400 ms) antes de teletransportar.
#define kDeathToBedSteps 4
// Rangos de id: los animales usan uint8 desde 1 y ViewMotion empareja por id
// en una sola lista, así que enemigos y llamas arrancan en rangos distintos.
#define kFirstEnemyId 1000
#define kFirstProjectileId 2000

// Dirección del paso más corto hacia (target_x, target_y): primero el eje con
// más distancia, para que la llama llegue en zigzag a cualquier celda.
static Direction direction_to(uint16_t x, uint16_t y, uint16_t target_x, uint16_t target_y) {
    const int dx = static_cast<int>(target_x) - static_cast<int>(x);
    const int dy = static_cast<int>(target_y) - static_cast<int>(y);
    if (std::abs(dx) >= std::abs(dy)) {
        return dx >= 0 ? Direction::RIGHT : Direction::LEFT;
    }
    return dy >= 0 ? Direction::DOWN : Direction::UP;
}

World::World(uint16_t w, uint16_t h):
        width(w),
        height(h),
        next_enemy_id(kFirstEnemyId),
        next_projectile_id(kFirstProjectileId),
        waking(false),
        day_ended(false),
        day_started(false),
        day(1) {
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
Player& World::get_player(uint8_t player_id) { return *this->players.at(player_id); }

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
            crops.erase(std::remove(crops.begin(), crops.end(), target_entity), crops.end());
            enemies.erase(std::remove(enemies.begin(), enemies.end(), target_entity),
                          enemies.end());
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
        remove_entity(cell_entities.front().get());
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
    const std::vector<Animal*> AToday = animals;
    for (Animal* animal: AToday) {
        animal->update(*this);
    }

    const std::vector<Crop*> crops_today = crops;
    for (Crop* crop: crops_today) {
        crop->update(*this);
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

void World::add_enemy(std::unique_ptr<Enemy> enemy) {
    const uint16_t x = enemy->get_x();
    const uint16_t y = enemy->get_y();
    if (x >= this->width || y >= this->height) {
        return;
    }
    // shortcut: id sin reciclar, se repite tras 65535 enemigos; upgrade: reusar ids libres.
    enemy->set_id(this->next_enemy_id++);
    set_entity_at(x, y, std::move(enemy));
    // set_entity_at agrega al final de la celda: el puntero es el de la celda, no el del parámetro.
    enemies.push_back(static_cast<Enemy*>(this->grid[y][x].entities.back().get()));
}

void World::spawn_orc_pack(uint16_t x, uint16_t y) {
    // Manada de 3 a 6 orcos en celdas caminables con radio <= 2 del punto:
    // 25 candidatas, así que casi siempre entra la manada completa.
    const int pack_size = kOrcPackMin + std::rand() % (kOrcPackMax - kOrcPackMin + 1);
    std::vector<std::pair<uint16_t, uint16_t>> cells;
    for (int radius = 0; radius <= kOrcPackRadius; ++radius) {
        for (int dy = -radius; dy <= radius; ++dy) {
            for (int dx = -radius; dx <= radius; ++dx) {
                if (std::max(std::abs(dx), std::abs(dy)) != radius) {
                    continue;  // Solo el anillo de este radio (evita repetir).
                }
                const int cell_x = static_cast<int>(x) + dx;
                const int cell_y = static_cast<int>(y) + dy;
                if (cell_x < 0 || cell_y < 0 || cell_x >= this->width || cell_y >= this->height) {
                    continue;
                }
                const uint16_t new_x = static_cast<uint16_t>(cell_x);
                const uint16_t new_y = static_cast<uint16_t>(cell_y);
                if (this->can_walk_to(new_x, new_y)) {
                    cells.emplace_back(new_x, new_y);
                }
            }
        }
    }
    // Arranque al azar para que la manada no aparezca siempre en el mismo orden.
    const size_t start = cells.empty() ? 0 : static_cast<size_t>(std::rand()) % cells.size();
    for (int i = 0; i < pack_size && static_cast<size_t>(i) < cells.size(); ++i) {
        const auto& [orc_x, orc_y] = cells[(start + i) % cells.size()];
        // Se re-chequea: el primer orco ya bloquea su propia celda.
        if (this->can_walk_to(orc_x, orc_y)) {
            this->add_enemy(std::make_unique<Orc>(orc_x, orc_y));
        }
    }
}

void World::spawn_vampire_pair(uint16_t x, uint16_t y) {
    // Pareja en dos celdas caminables con radio <= 1 del punto (hasta 9
    // candidatas): con menos de 2 no hay pareja.
    std::vector<std::pair<uint16_t, uint16_t>> cells;
    for (int dy = -kVampirePairRadius; dy <= kVampirePairRadius; ++dy) {
        for (int dx = -kVampirePairRadius; dx <= kVampirePairRadius; ++dx) {
            const int cell_x = static_cast<int>(x) + dx;
            const int cell_y = static_cast<int>(y) + dy;
            if (cell_x < 0 || cell_y < 0 || cell_x >= this->width || cell_y >= this->height) {
                continue;
            }
            const uint16_t new_x = static_cast<uint16_t>(cell_x);
            const uint16_t new_y = static_cast<uint16_t>(cell_y);
            if (this->can_walk_to(new_x, new_y)) {
                cells.emplace_back(new_x, new_y);
            }
        }
    }
    if (cells.size() < kVampirePairMinCells) {
        return;
    }
    // Arranque al azar para que no aparezca siempre en el mismo orden.
    const size_t start = static_cast<size_t>(std::rand()) % cells.size();
    const auto& [first_x, first_y] = cells[start];
    const auto& [second_x, second_y] = cells[(start + 1) % cells.size()];
    // Los dos se construyen antes de agregar: nadie bloquea la celda del otro.
    auto vampire = std::make_unique<Vampire>(first_x, first_y);
    auto vampiress = std::make_unique<Vampiress>(second_x, second_y);
    Vampire* vampire_ptr = vampire.get();
    Vampiress* vampiress_ptr = vampiress.get();
    vampire_ptr->set_partner(vampiress_ptr);
    vampiress_ptr->set_partner(vampire_ptr);
    this->add_enemy(std::move(vampire));
    this->add_enemy(std::move(vampiress));
}

void World::clear_enemies() {
    // Se copian los punteros: remove_entity borra del vector mientras se itera.
    const std::vector<Enemy*> all = this->enemies;
    for (Enemy* enemy: all) {
        this->remove_entity(enemy);
    }
}

void World::clear_flames() { this->flames.clear(); }

void World::tick_enemies() {
    // Se copian los punteros: update puede agregar enemigos (spawn) y rearmar el vector.
    const std::vector<Enemy*> today = enemies;
    for (Enemy* enemy: today) {
        enemy->update(*this);
    }
}

bool World::tick_enemies_pursuit() {
    // Se copian los punteros: la IA puede cambiar listas (muerte, spawn).
    const std::vector<Enemy*> today = enemies;
    bool changed = false;
    for (Enemy* enemy: today) {
        if (enemy->get_health() == 0) {
            continue;  // Los cadáveres no persiguen.
        }
        changed = enemy->pursue(*this) || changed;
    }
    return changed;
}

void World::spawn_flame(uint16_t x, uint16_t y, uint16_t target_x, uint16_t target_y,
                        uint8_t damage, const std::string& kind) {
    const Direction direction = direction_to(x, y, target_x, target_y);
    this->flames.push_back(
            Flame{this->next_projectile_id++, kind, x, y, direction, damage, target_x, target_y});
}

bool World::damage_players_at(uint16_t x, uint16_t y, uint8_t damage) {
    for (const uint8_t id: this->get_player_ids()) {
        const Player& player = this->get_player(id);
        if (player.get_health() > 0 && player.get_x() == x && player.get_y() == y) {
            this->damage_player(id, damage);
            return true;
        }
    }
    return false;
}

bool World::tick_flames() {
    if (this->flames.empty()) {
        return false;
    }
    auto it = this->flames.begin();
    while (it != this->flames.end()) {
        Flame& flame = *it;
        // Contacto: el jugador puede haber entrado en la celda de la llama.
        if (this->damage_players_at(flame.x, flame.y, flame.damage)) {
            it = this->flames.erase(it);
            continue;
        }
        const Direction direction = direction_to(flame.x, flame.y, flame.target_x, flame.target_y);
        const auto [next_x, next_y] = IMovable::next_tile(flame.x, flame.y, direction);
        if ((next_x == flame.x && next_y == flame.y) || !this->can_walk_to(next_x, next_y)) {
            it = this->flames.erase(it);  // Borde del mapa o pared: se apaga.
            continue;
        }
        flame.x = next_x;
        flame.y = next_y;
        flame.direction = direction;
        if (this->damage_players_at(flame.x, flame.y, flame.damage) ||
            (flame.x == flame.target_x && flame.y == flame.target_y)) {
            it = this->flames.erase(it);
            continue;
        }
        ++it;
    }
    return true;
}

const std::vector<std::pair<uint16_t, uint16_t>>& World::blood_puddle_list() const {
    return this->blood_puddles;
}

void World::clear_blood_puddle() { this->blood_puddles.clear(); }

std::vector<ProjectileSnapshot> World::projectile_snapshots() const {
    std::vector<ProjectileSnapshot> out;
    out.reserve(this->flames.size());
    for (const Flame& flame: this->flames) {
        out.push_back(
                ProjectileSnapshot{flame.id, flame.kind, flame.x, flame.y, flame.direction});
    }
    return out;
}

bool World::damage_player(uint8_t player_id, uint8_t damage) {
    Player& player = *this->players.at(player_id);
    const uint16_t hp_before = player.get_health();
    const bool killed = player.take_damage(damage);
    // Charco de sangre: solo el golpe que dejó al jugador por debajo del 20 %
    // (incluida la muerte) deja sangre; golpear a un ya caído no suma.
    if (player.get_health() < hp_before && player.is_gravely_wounded()) {
        this->blood_puddles.emplace_back(player.get_x(), player.get_y());
    }
    if (killed) {
        this->kill_player(player_id);
    }
    return killed;
}

bool World::consume_blood_puddle() {
    if (this->blood_puddles.empty()) {
        return false;
    }
    this->blood_puddles.pop_back();
    return true;
}

bool World::has_blood_puddle() const {
    return !this->blood_puddles.empty();
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
    if (p.is_energy_depleted()) return;
    auto [tx, ty] = p.get_target_tile();

    if (tx >= this->width || ty >= this->height) return;
    Cell& target_cell = this->grid[ty][tx];
    ToolType tool = p.get_equipped_tool();

    bool action_performed = false;

    if (tool == ToolType::SHOVEL) {
        target_cell.till_soil();
        action_performed = true; 
    } 
    else if (tool == ToolType::WATERING_CAN) {
        target_cell.water_soil();
        action_performed = true;
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
        action_performed = true;
    } else if (tool == ToolType::HAND) {
        action_performed = this->try_plant_seed(player_id, tx, ty);
    }

    if (tool != ToolType::SWORD && tool != ToolType::HAND) {
        apply_consume_energy(player_id, action_performed ? 5.0f : 1.0f);
    }
}

void World::apply_consume_energy(uint8_t player_id, float amount) {
    Player& p = *this->players.at(player_id);
    p.consume_energy(amount);
    if (p.is_energy_depleted()) {
        this->faint_player(player_id);
    }
}

bool World::try_plant_seed(uint8_t player_id, uint16_t tx, uint16_t ty) {
    // TODO: Se parece mucho a use_tool, refactorizar para no repetir código.
    Player& p = *this->players.at(player_id);
    Cell& target_cell = this->grid[ty][tx];

    if (!target_cell.is_soil_tilled() && !target_cell.is_soil_watered()) return false;

    const Item* active_item = p.peek_equipped_item();
    if (!active_item) return false; 

    //TODO: Sacar este switch a un factory method en CropFactory para no tener que tocar World.cpp cada vez que agregamos un nuevo cultivo.
    std::unique_ptr<Crop> new_crop = nullptr;
    switch (active_item->get_id()) {
        case ItemID::STRAWBERRY_SEED: new_crop = std::make_unique<Strawberry>(tx, ty); break;
        case ItemID::ONION_SEED: new_crop = std::make_unique<GreenOnion>(tx, ty); break;
        case ItemID::POTATO_SEED: new_crop = std::make_unique<Potato>(tx, ty); break;
        case ItemID::PUMPKIN_SEED: new_crop = std::make_unique<Pumpkin>(tx, ty); break;
        default: return false; 
    }
    
    crops.push_back(new_crop.get());
    if (new_crop) {
        p.consume_equipped_item(); 
        target_cell.entities.push_back(std::move(new_crop));
        // Si se planta una semilla, el suelo queda arado aunque se haya regado antes.
        target_cell.till_soil();
        return true; 
    }
    return false;
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

void World::select_slot(uint8_t player_id, uint8_t slot) {
    this->players.at(player_id)->select_slot(slot);
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
                // cppcheck-suppress useStlAlgorithm; interact consume la acción y corta el loop.
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
    // La sangre se seca al terminar el día: los vampiros no revive con
    // charcos viejos.
    this->blood_puddles.clear();
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

const Cell& World::get_cell(uint16_t x, uint16_t y) const {
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
    this->death_to_bed.erase(player_id);
    // Si el que se fue era el único que faltaba aceptar o despertar, el día tiene que avanzar.
    this->try_advance_sleep();
    this->try_start_day();
}

void World::set_player_position(uint8_t player_id, uint16_t x, uint16_t y) {
    this->players.at(player_id)->set_position(x, y);
}

void World::kill_player(uint8_t player_id) {
    Player& player = *this->players.at(player_id);
    player.set_health(0);
    player.setDiedInCombat(true);
    // La cama y el sueño quedan diferidos: primero el desmayo en el
    // lugar de la muerte (tick_death_to_bed los despacha).
    // shortcut: mientras espera, el muerto sigue pudiendo mandar
    // comandos (bloquear movimiento de muertos queda fuera de alcance);
    // en ≤1,6 s ya está en la cama, y en sleepers no se mueve.
    this->death_to_bed.emplace(player_id, kDeathToBedSteps);
}

void World::faint_player(uint8_t player_id) {
    Player& player = *this->players.at(player_id);
    // Desmayo por falta de energía: la vida queda intacta, pero el flujo
    // (teletransporte a la cama, sueño no cancelable, penalización al
    // amanecer) es el mismo que la muerte en combate. El cliente se entera
    // por el flag `fainted` de Status (energía 0 es el disparador).
    player.setFaintedByEnergy(true);
    this->death_to_bed.emplace(player_id, kDeathToBedSteps);
}
bool World::tick_death_to_bed() {
    bool arrived = false;
    for (auto it = this->death_to_bed.begin(); it != this->death_to_bed.end();) {
        const uint8_t id = it->first;
        if (it->second > 0) {
            --it->second;
            ++it;
            continue;
        }
        // Venció la espera: remove_player limpia death_to_bed, así que
        // el id sigue vivo acá.
        it = this->death_to_bed.erase(it);
        Player& player = *this->players.at(id);
        // Revivió (amaneció) mientras esperaba: no hay cama que buscar.
        // Un desmayado conserva la vida, así que también hay que mirar el
        // flag: si ya no está desmayado, saltearlo.
        if (player.get_health() > 0 && !player.faintedByEnergy()) {
            continue;
        }
        player.set_position(20, 13);  // misma cama que kBedX/kBedY (match.cpp)
        this->record_sleep_request(id);
        this->answer_sleep(id, true);
        arrived = true;
    }
    return arrived;
}

void World::mark_all_fainted_by_timeout() {
    for (const uint8_t id: this->get_player_ids()) {
        this->players.at(id)->setFaintedByTimeout(true);
    }
}

void World::reset_player_for_day(uint8_t player_id) {
    Player& p = *this->players.at(player_id);
    const bool penalized = p.diedInCombat() || p.faintedByTimeout() || p.faintedByEnergy();
    const uint16_t new_hp = penalized ? static_cast<uint16_t>(p.get_max_health() / 2)
                                     : p.get_max_health();
    const float new_energy = penalized ? (p.get_max_energy() / 2.0f) : p.get_max_energy();
    p.set_health(new_hp);
    p.set_energy(new_energy);
    p.setDiedInCombat(false);
    p.setFaintedByTimeout(false);
    p.setFaintedByEnergy(false);
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
        // Un jugador muerto no puede levantarse: se queda dormido hasta el amanecer.
        if (this->players.at(player_id)->diedInCombat() ||
            this->players.at(player_id)->faintedByEnergy()) {
            this->accepted.insert(player_id);
            this->try_advance_sleep();
            return;
        }
        // Cancelar: sale de la cama...
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
