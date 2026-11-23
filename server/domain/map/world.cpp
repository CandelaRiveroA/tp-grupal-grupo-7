#include "world.h"

#include <algorithm>
#include <cstdlib>
#include <stdexcept>
#include <utility>

#include "../animals/animal.h"
#include "../enemies/enemy.h"
#include "../npc/merchant.h"

// Ciclo fijo de 28 días que vuelve al día 1.
#define kDaysPerCycle 28
// Cantidad de salas de la mazmorra (DUNGEON_1..DUNGEON_10).
#define kDungeonFloorCount 10
// Pasos de EnemyTick (400 ms) entre la muerte/desmayo y la cama.
#define kDeathToBedSteps 4
// Rangos de id: animales desde 1, enemigos desde 1000, proyectiles desde 2000.
#define kFirstEnemyId 1000
#define kFirstProjectileId 2000
// Coordenadas de la cama en FARMHOUSE (coincide con kBedX/kBedY de match.cpp).
#define kBedX 20
#define kBedY 13

static SceneId dungeon_scene_id(uint8_t room_number) {
    return static_cast<SceneId>(static_cast<uint8_t>(SceneId::DUNGEON_1) + (room_number - 1));
}

static bool is_dungeon_scene(SceneId id) {
    return static_cast<uint8_t>(id) >= static_cast<uint8_t>(SceneId::DUNGEON_1) &&
           static_cast<uint8_t>(id) <= static_cast<uint8_t>(SceneId::DUNGEON_10);
}

World::World(uint16_t w, uint16_t h):
        scenes(),
        farmhouse(nullptr),
        farm(nullptr),
        town(nullptr),
        dungeon_floors(),
        active_scene(SceneId::FARMHOUSE),
        selected_merchants(),
        next_enemy_id(kFirstEnemyId),
        next_projectile_id(kFirstProjectileId),
        next_orc_group_id(1),
        next_vampire_pair_id(1),
        resource_hits(),
        pickups(),
        gift_answers(),
        pending_gifts(),
        sleepers(),
        accepted(),
        death_to_bed(),
        waking(false),
        woke(),
        day_ended(false),
        day_started(false),
        day(1),
        community_center(),
        community_victory(false),
        current_room(0),
        market_discount_active(false),
        double_sell_active(false),
        skull_survived_without_damage(true),
        community_set_catalog(),
        community_set_rewards(),
        players() {
    auto house_ptr = std::make_unique<Scene>(w, h);
    this->farmhouse = house_ptr.get();
    this->scenes.emplace(SceneId::FARMHOUSE, std::move(house_ptr));

    auto farm_ptr = std::make_unique<Farm>(w, h);
    this->farm = farm_ptr.get();
    this->scenes.emplace(SceneId::FARM, std::move(farm_ptr));

    auto town_ptr = std::make_unique<Town>(w, h);
    this->town = town_ptr.get();
    this->scenes.emplace(SceneId::TOWN, std::move(town_ptr));

    this->dungeon_floors.reserve(kDungeonFloorCount);
    const uint16_t default_spawn_x = w / 2;
    const uint16_t default_spawn_y = h / 2;
    for (uint8_t room = 1; room <= kDungeonFloorCount; ++room) {
        auto floor_ptr = std::make_unique<DungeonFloor>(room, w, h,
                                                        default_spawn_x, default_spawn_y);
        this->dungeon_floors.push_back(floor_ptr.get());
        this->scenes.emplace(dungeon_scene_id(room), std::move(floor_ptr));
    }
}

// ---------------------------------------------------------
// GESTIÓN DE ESCENARIOS
// ---------------------------------------------------------
Scene& World::get_scene(SceneId id) { return *this->scenes.at(id); }

const Scene& World::get_scene(SceneId id) const { return *this->scenes.at(id); }

const Scene& World::get_scene_of(uint8_t player_id) const {
    return get_scene(this->players.at(player_id)->get_scene());
}

void World::set_active_scene(SceneId id) { this->active_scene = id; }

Farm& World::get_farm() { return *this->farm; }

Town& World::get_town() { return *this->town; }

DungeonFloor& World::get_dungeon_floor(uint8_t room_number) {
    if (room_number < 1 || room_number > this->dungeon_floors.size()) {
        throw std::out_of_range("Dungeon room number out of bounds");
    }
    return *this->dungeon_floors[room_number - 1];
}

DungeonFloor* World::active_dungeon_floor() {
    if (!is_dungeon_scene(this->active_scene)) {
        return nullptr;
    }
    const size_t idx = static_cast<size_t>(static_cast<uint8_t>(this->active_scene) -
                                           static_cast<uint8_t>(SceneId::DUNGEON_1));
    return this->dungeon_floors[idx];
}

const DungeonFloor* World::active_dungeon_floor() const {
    if (!is_dungeon_scene(this->active_scene)) {
        return nullptr;
    }
    const size_t idx = static_cast<size_t>(static_cast<uint8_t>(this->active_scene) -
                                           static_cast<uint8_t>(SceneId::DUNGEON_1));
    return this->dungeon_floors[idx];
}

void World::transfer_player(uint8_t player_id, SceneId target_scene, uint16_t x, uint16_t y) {
    Player& player = *this->players.at(player_id);
    if (!get_scene(target_scene).in_bounds(x, y)) {
        return;
    }

    if (player.get_scene() != target_scene) {
        this->selected_merchants.erase(player_id);
    }
    player.set_scene(target_scene);
    player.set_position(x, y);

    if (is_dungeon_scene(target_scene)) {
        const uint8_t room = static_cast<uint8_t>(static_cast<uint8_t>(target_scene) -
                                                  static_cast<uint8_t>(SceneId::DUNGEON_1) + 1);
        set_current_room(room);
    } else if (this->current_room != 0) {
        leave_dungeon();
    }
}

// ---------------------------------------------------------
// FACHADA ESPACIAL SOBRE active_scene
// ---------------------------------------------------------
bool World::can_walk_to(uint16_t x, uint16_t y) const {
    return get_scene(this->active_scene).can_walk_to(x, y);
}

uint16_t World::get_width() const { return get_scene(this->active_scene).get_width(); }

uint16_t World::get_height() const { return get_scene(this->active_scene).get_height(); }

SoilState World::get_soil_state(uint16_t x, uint16_t y) const {
    return get_scene(this->active_scene).get_soil_state(x, y);
}

const std::vector<IEntity*> World::get_entity_at(uint16_t x, uint16_t y) const {
    return get_scene(this->active_scene).get_entity_at(x, y);
}

void World::set_entity_at(uint16_t x, uint16_t y, std::unique_ptr<IEntity> new_entity) {
    get_scene(this->active_scene).set_entity_at(x, y, std::move(new_entity));
}

void World::remove_entity(IEntity* target_entity) {
    get_scene(this->active_scene).remove_entity(target_entity);
}

void World::remove_entity_at(uint16_t x, uint16_t y) {
    get_scene(this->active_scene).remove_entity_at(x, y);
}

void World::move_entity(IEntity* entity, uint16_t x, uint16_t y) {
    get_scene(this->active_scene).move_entity(entity, x, y);
}

// ---------------------------------------------------------
// GESTIÓN DE JUGADORES Y MOVIMIENTO
// ---------------------------------------------------------
void World::add_player(uint8_t player_id, std::unique_ptr<Player> player) {
    this->players[player_id] = std::move(player);
}

void World::remove_player(uint8_t player_id) {
    this->players.erase(player_id);
    this->selected_merchants.erase(player_id);
    this->sleepers.erase(player_id);
    this->accepted.erase(player_id);
    this->woke.erase(player_id);
    this->death_to_bed.erase(player_id);
    this->try_advance_sleep();
    this->try_start_day();
}

void World::move_player(uint8_t player_id, Direction dir) {
    Player& p = *this->players.at(player_id);

    // Durmiendo no se mueve: solo cancelar le devuelve el movimiento.
    if (this->sleepers.count(player_id) > 0) {
        return;
    }

    this->active_scene = p.get_scene();
    Scene& scene = get_scene(this->active_scene);

    p.set_facing(dir);
    for (uint8_t step = 0; step < p.get_movement_steps(); ++step) {
        auto [next_x, next_y] = p.get_target_tile();
        if (!scene.can_walk_to(next_x, next_y)) {
            break;
        }

        p.set_position(next_x, next_y);

        // Efecto imán / Auto-Pickup sobre entidades pisables de la celda.
        const std::vector<IEntity*> entities = scene.get_entity_at(next_x, next_y);
        for (IEntity* ent: entities) {
            if (ent->pickup_on_take()) {
                continue;
            }
            ent->interact(p, ToolType::HAND, *this);
            if (p.get_scene() != this->active_scene) {
                break;
            }
        }

        // Si la interacción cambió de escena (ej. puerta) o acostó al jugador, cortar paso.
        if (this->is_sleeping(player_id) || p.get_scene() != this->active_scene) {
            break;
        }
    }
}

void World::toggle_player_walking(uint8_t player_id) {
    this->players.at(player_id)->toggle_walking();
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

std::vector<uint8_t> World::get_player_ids_in_scene(SceneId scene_id) const {
    std::vector<uint8_t> ids;
    for (const auto& [id, player]: this->players) {
        if (player->get_scene() == scene_id) {
            ids.push_back(id);
        }
    }
    return ids;
}

void World::set_player_position(uint8_t player_id, uint16_t x, uint16_t y) {
    this->players.at(player_id)->set_position(x, y);
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

// ---------------------------------------------------------
// ANIMALES (DELEGADOS A FARM)
// ---------------------------------------------------------
void World::add_animal(std::unique_ptr<Animal> animal) {
    this->farm->add_animal(std::move(animal));
}

void World::tick_animals() {
    this->active_scene = SceneId::FARM;
    this->farm->tick_animals(*this);
}

// ---------------------------------------------------------
// COMERCIANTES (DELEGADOS A TOWN)
// ---------------------------------------------------------
void World::place_merchant(std::unique_ptr<Merchant> merchant) {
    if (!merchant) {
        return;
    }
    this->town->place_merchant(std::move(merchant));
}

Merchant* World::get_nearest_merchant(uint8_t player_id) const {
    const Player& player = get_player(player_id);
    if (player.get_scene() != SceneId::TOWN) {
        return nullptr;
    }
    return this->town->get_nearest_merchant(player.get_x(), player.get_y());
}

std::vector<MerchantPlacement> World::get_merchant_placements() const {
    return this->town->get_merchant_placements();
}

Merchant* World::get_merchant_for_command(uint8_t player_id) {
    const Player& player = get_player(player_id);
    if (player.get_scene() != SceneId::TOWN) {
        this->selected_merchants.erase(player_id);
        return nullptr;
    }
    const auto selected = this->selected_merchants.find(player_id);
    if (selected != this->selected_merchants.end()) {
        Merchant* merchant = selected->second;
        const int dx = std::abs(static_cast<int>(merchant->get_x()) -
                                static_cast<int>(player.get_x()));
        const int dy = std::abs(static_cast<int>(merchant->get_y()) -
                                static_cast<int>(player.get_y()));
        if (dx <= 1 && dy <= 1) {
            return merchant;
        }
        this->selected_merchants.erase(selected);
    }
    return get_nearest_merchant(player_id);
}

std::string World::process_merchant_command(uint8_t player_id, const std::string& text) {
    Merchant* merchant = get_merchant_for_command(player_id);
    if (merchant == nullptr) {
        return "No hay ningun comerciante cerca.";
    }
    this->active_scene = get_player(player_id).get_scene();
    return merchant->process_command(get_player(player_id), text, *this);
}

// ---------------------------------------------------------
// ENEMIGOS, LLAMAS Y SANGRE (DELEGADOS A DUNGEON FLOOR)
// ---------------------------------------------------------
void World::add_enemy(std::unique_ptr<Enemy> enemy) {
    DungeonFloor* floor = active_dungeon_floor();
    if (!floor) {
        return;
    }
    floor->add_enemy(std::move(enemy), this->next_enemy_id++);
}

void World::spawn_orc_pack(uint16_t x, uint16_t y) {
    DungeonFloor* floor = active_dungeon_floor();
    if (!floor) {
        return;
    }
    floor->spawn_orc_pack(x, y, this->next_enemy_id, this->next_orc_group_id++);
}

void World::spawn_vampire_pair(uint16_t x, uint16_t y) {
    DungeonFloor* floor = active_dungeon_floor();
    if (!floor) {
        return;
    }
    floor->spawn_vampire_pair(x, y, this->next_enemy_id, this->next_vampire_pair_id++);
}

void World::clear_enemies() {
    if (DungeonFloor* floor = active_dungeon_floor()) {
        floor->clear_enemies();
    }
}

void World::clear_flames() {
    if (DungeonFloor* floor = active_dungeon_floor()) {
        floor->clear_flames();
    }
}

void World::tick_enemies() {
    for (DungeonFloor* floor: this->dungeon_floors) {
        this->active_scene = floor->get_scene_id();
        floor->tick_enemies(*this);
    }
}

bool World::tick_enemies_pursuit() {
    bool changed = false;
    for (DungeonFloor* floor: this->dungeon_floors) {
        this->active_scene = floor->get_scene_id();
        changed = floor->tick_enemies_pursuit(*this) || changed;
    }
    return changed;
}

bool World::tick_flames() {
    bool any_flying = false;
    for (DungeonFloor* floor: this->dungeon_floors) {
        this->active_scene = floor->get_scene_id();
        any_flying = floor->tick_flames(*this) || any_flying;
    }
    return any_flying;
}

void World::spawn_flame(uint16_t x, uint16_t y, uint16_t target_x, uint16_t target_y,
                        uint8_t damage, const std::string& kind) {
    DungeonFloor* floor = active_dungeon_floor();
    if (!floor) {
        return;
    }
    floor->spawn_flame(this->next_projectile_id++, x, y, target_x, target_y, damage, kind);
}

std::vector<ProjectileSnapshot> World::projectile_snapshots() const {
    return get_scene(this->active_scene).projectile_snapshots();
}

const std::vector<std::pair<uint16_t, uint16_t>>& World::blood_puddle_list() const {
    return get_scene(this->active_scene).blood_puddle_list();
}

void World::clear_blood_puddle() {
    if (DungeonFloor* floor = active_dungeon_floor()) {
        floor->clear_blood_puddle();
    }
}

bool World::consume_blood_puddle() {
    DungeonFloor* floor = active_dungeon_floor();
    return floor ? floor->consume_blood_puddle() : false;
}

bool World::has_blood_puddle() const {
    const DungeonFloor* floor = active_dungeon_floor();
    return floor ? floor->has_blood_puddle() : false;
}

// ---------------------------------------------------------
// COMBATE, DAÑO Y DESMAYO
// ---------------------------------------------------------
bool World::damage_players_at(SceneId scene_id, uint16_t x, uint16_t y, uint8_t damage) {
    for (const uint8_t id: get_player_ids_in_scene(scene_id)) {
        const Player& player = get_player(id);
        if (player.get_health() > 0 && player.get_x() == x && player.get_y() == y) {
            damage_player(id, damage);
            return true;
        }
    }
    return false;
}

bool World::damage_player(uint8_t player_id, uint8_t damage) {
    Player& player = *this->players.at(player_id);
    const uint16_t hp_before = player.get_health();
    this->skull_survived_without_damage = false;
    const bool killed = player.take_damage(damage);

    if (player.get_health() < hp_before && player.is_gravely_wounded() &&
        is_dungeon_scene(player.get_scene())) {
        const uint8_t room = static_cast<uint8_t>(static_cast<uint8_t>(player.get_scene()) -
                                                  static_cast<uint8_t>(SceneId::DUNGEON_1) + 1);
        get_dungeon_floor(room).add_blood_puddle(player.get_x(), player.get_y());
    }
    if (killed) {
        kill_player(player_id);
    }
    return killed;
}

void World::kill_player(uint8_t player_id) {
    Player& player = *this->players.at(player_id);
    player.set_health(0);
    player.setDiedInCombat(true);
    this->death_to_bed.emplace(player_id, kDeathToBedSteps);
}

void World::faint_player(uint8_t player_id) {
    Player& player = *this->players.at(player_id);
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
        it = this->death_to_bed.erase(it);
        Player& player = *this->players.at(id);
        if (player.get_health() > 0 && !player.faintedByEnergy()) {
            continue;
        }
        transfer_player(id, SceneId::FARMHOUSE, kBedX, kBedY);
        record_sleep_request(id);
        answer_sleep(id, true);
        arrived = true;
    }
    return arrived;
}

void World::mark_all_fainted_by_timeout() {
    for (const uint8_t id: get_player_ids()) {
        this->players.at(id)->setFaintedByTimeout(true);
    }
}

void World::reset_player_for_day(uint8_t player_id) {
    Player& p = *this->players.at(player_id);
    const bool penalized = p.diedInCombat() || p.faintedByTimeout() || p.faintedByEnergy();
    const uint16_t new_hp = penalized ? static_cast<uint16_t>(p.get_max_health() / 2)
                                      : p.get_max_health();
    const uint16_t new_energy = penalized ? static_cast<uint16_t>(p.get_max_energy() / 2)
                                          : p.get_max_energy();
    p.set_health(new_hp);
    p.set_energy(new_energy);
    p.setDiedInCombat(false);
    p.setFaintedByTimeout(false);
    p.setFaintedByEnergy(false);
}

// ---------------------------------------------------------
// ACCIONES DEL JUGADOR
// ---------------------------------------------------------
void World::use_tool(uint8_t player_id) {
    if (this->sleepers.count(player_id) > 0) {
        return;
    }
    Player& p = *this->players.at(player_id);
    if (p.is_energy_depleted()) {
        return;
    }

    this->active_scene = p.get_scene();
    Scene& scene = get_scene(this->active_scene);
    auto [tx, ty] = p.get_target_tile();
    if (!scene.in_bounds(tx, ty)) {
        return;
    }

    const ToolType tool = p.get_equipped_tool();
    bool action_performed = false;

    // Solo en la Granja se puede arar o regar el suelo.
    if (this->active_scene == SceneId::FARM) {
        if (tool == ToolType::SHOVEL) {
            this->farm->till_soil(tx, ty);
            action_performed = true;
        } else if (tool == ToolType::WATERING_CAN) {
            this->farm->water_soil(tx, ty);
            action_performed = true;
        }
    }

    const std::vector<IEntity*> entities = scene.get_entity_at(tx, ty);
    if (!entities.empty()) {
        for (IEntity* ent: entities) {
            ent->interact(p, tool, *this);
        }
        action_performed = true;
    } else if (tool == ToolType::HAND) {
        action_performed = try_plant_seed(player_id, tx, ty);
    }

    if (tool != ToolType::SWORD && tool != ToolType::HAND) {
        apply_consume_energy(player_id, action_performed ? 500 : 100);
    }
}

void World::apply_consume_energy(uint8_t player_id, uint16_t amount) {
    Player& p = *this->players.at(player_id);
    p.consume_energy(amount);
    if (p.is_energy_depleted()) {
        faint_player(player_id);
    }
}

bool World::try_plant_seed(uint8_t player_id, uint16_t tx, uint16_t ty) {
    Player& p = *this->players.at(player_id);
    if (p.get_scene() != SceneId::FARM) {
        return false;
    }
    return this->farm->try_plant_seed(p, tx, ty);
}

void World::take_item(uint8_t player_id) {
    if (this->sleepers.count(player_id) > 0) {
        return;
    }
    Player& p = *this->players.at(player_id);
    this->active_scene = p.get_scene();
    Scene& scene = get_scene(this->active_scene);

    // Comerciantes (solo en TOWN).
    if (Merchant* merchant = get_nearest_merchant(player_id)) {
        this->selected_merchants[player_id] = merchant;
        merchant->interact(p, ToolType::HAND, *this);
        return;
    }

    // Ordeñe / animales (solo en FARM).
    if (this->active_scene == SceneId::FARM &&
        this->farm->try_hand_action_animals(p, *this)) {
        return;
    }

    // Huevos u otros ítems con pickup_on_take en las 8 vecinas.
    for (int dy = -1; dy <= 1; ++dy) {
        for (int dx = -1; dx <= 1; ++dx) {
            const auto x = static_cast<uint16_t>(p.get_x() + dx);
            const auto y = static_cast<uint16_t>(p.get_y() + dy);
            for (IEntity* ent: scene.get_entity_at(x, y)) {
                if (ent->pickup_on_take()) {
                    ent->interact(p, ToolType::HAND, *this);
                    return;
                }
            }
        }
    }

    // Entidades en la celda propia.
    for (IEntity* ent: scene.get_entity_at(p.get_x(), p.get_y())) {
        ent->interact(p, ToolType::HAND, *this);
    }

    // Regalo a otro jugador en la misma escena y radio <= 1.
    for (const auto& [id, other]: this->players) {
        if (id == player_id || other->get_scene() != this->active_scene) {
            continue;
        }
        const int dx = static_cast<int>(other->get_x()) - static_cast<int>(p.get_x());
        const int dy = static_cast<int>(other->get_y()) - static_cast<int>(p.get_y());
        if (std::abs(dx) <= 1 && std::abs(dy) <= 1) {
            other->interact(p, ToolType::HAND, *this);
            return;
        }
    }
}

void World::record_resource_hit(uint16_t x, uint16_t y, bool destroyed) {
    this->resource_hits.push_back(ResourceHit{x, y, destroyed});
}

std::vector<ResourceHit> World::take_resource_hits() {
    std::vector<ResourceHit> hits;
    hits.swap(this->resource_hits);
    return hits;
}

void World::record_pickup(uint8_t player_id, ItemID item) {
    this->pickups.push_back(PlayerPickup{player_id, item});
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

std::vector<PlayerPickup> World::take_pickups() {
    std::vector<PlayerPickup> taken;
    taken.swap(this->pickups);
    return taken;
}

// ---------------------------------------------------------
// DÍA Y SUEÑO
// ---------------------------------------------------------
void World::tick_day() {
    for (auto& [scene_id, scene_ptr]: this->scenes) {
        this->active_scene = scene_id;
        scene_ptr->on_new_day(*this);
    }
}

void World::advance_day() {
    this->day = static_cast<uint8_t>(this->day % kDaysPerCycle + 1);
    tick_day();
}

uint8_t World::get_day() const { return this->day; }

void World::clear_sleep() {
    this->sleepers.clear();
    this->accepted.clear();
}

void World::record_sleep_request(uint8_t player_id) {
    if (this->waking || this->sleepers.count(player_id) > 0) {
        return;
    }
    this->sleepers.insert(player_id);
}

void World::answer_sleep(uint8_t player_id, bool accept) {
    if (this->sleepers.count(player_id) == 0) {
        return;
    }

    if (!accept) {
        if (this->players.at(player_id)->diedInCombat() ||
            this->players.at(player_id)->faintedByEnergy()) {
            this->accepted.insert(player_id);
            try_advance_sleep();
            return;
        }
        this->sleepers.erase(player_id);
        this->accepted.erase(player_id);
        return;
    }

    this->accepted.insert(player_id);
    try_advance_sleep();
}

void World::try_advance_sleep() {
    if (this->players.empty() || this->sleepers.size() < this->players.size() ||
        this->accepted.size() < this->players.size()) {
        return;
    }
    end_day();
}

void World::end_day() {
    if (this->waking) {
        return;
    }
    clear_sleep();
    this->waking = true;
    this->woke.clear();
    this->day_ended = true;
}

void World::answer_wake(uint8_t player_id) {
    if (!this->waking || this->players.count(player_id) == 0) {
        return;
    }
    this->woke.insert(player_id);
    try_start_day();
}

void World::try_start_day() {
    if (!this->waking || this->players.empty() || this->woke.size() < this->players.size()) {
        return;
    }
    this->waking = false;
    this->woke.clear();
    advance_day();
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

// ---------------------------------------------------------
// CENTRO COMUNITARIO
// ---------------------------------------------------------
CommunityCenter& World::get_community_center() { return this->community_center; }

const CommunityCenter& World::get_community_center() const { return this->community_center; }

void World::set_current_room(uint8_t room) {
    if (room == 0) {
        this->skull_survived_without_damage = false;
    } else if (this->current_room == 0) {
        this->skull_survived_without_damage = true;
    } else if (room == this->current_room + 1 && this->skull_survived_without_damage) {
        this->community_center.record_skull_survival(true);
        if (this->community_center.try_complete_if_ready(CommunitySet::COMBAT)) {
            complete_set(CommunitySet::COMBAT);
        }
    } else {
        this->skull_survived_without_damage = false;
    }
    this->current_room = room;
}

void World::leave_dungeon() {
    this->skull_survived_without_damage = false;
    this->current_room = 0;
}

uint8_t World::get_current_room() const { return this->current_room; }

CommunitySet World::community_set_from_name(const std::string& set_name) const {
    return this->community_set_catalog.set_for_name(set_name);
}

void World::apply_set_reward(CommunitySet set) {
    this->community_set_rewards.apply(*this, set);
}

bool World::complete_set(const std::string& set_name) {
    const CommunitySet set = community_set_from_name(set_name);
    if (static_cast<uint8_t>(set) == 0) {
        return false;
    }
    return complete_set(set);
}

bool World::complete_set(CommunitySet set) {
    if (static_cast<uint8_t>(set) < 1 || static_cast<uint8_t>(set) > 5) {
        return false;
    }
    if (!this->community_center.complete_set(set)) {
        return false;
    }
    apply_set_reward(set);
    if (this->community_center.is_community_center_completed()) {
        this->community_victory = true;
    }
    return true;
}

bool World::complete_all_sets() {
    bool changed = false;
    for (const std::string& set_name: this->community_set_catalog.all_names()) {
        changed = complete_set(set_name) || changed;
    }
    return changed;
}

bool World::is_set_completed(const std::string& set_name) const {
    const CommunitySet set = community_set_from_name(set_name);
    return static_cast<uint8_t>(set) != 0 && this->community_center.is_set_completed(set);
}

bool World::is_community_center_completed() const {
    return this->community_center.is_community_center_completed();
}

bool World::donate_to_community_center(uint8_t player_id, const std::string& set_name,
                                       ItemID item, uint16_t quantity) {
    Player& p = get_player(player_id);
    const CommunitySet set = community_set_from_name(set_name);
    if (static_cast<uint8_t>(set) != 0 && this->community_center.is_set_completed(set)) {
        return false;
    }

    if (set == CommunitySet::MONEY) {
        const uint32_t missing =
                10000u - static_cast<uint32_t>(this->community_center.get_gold_donated());
        if (missing == 0) {
            return false;
        }
        if (quantity > missing) {
            quantity = static_cast<uint16_t>(missing);
        }
        if (!p.try_remove_gold(quantity)) {
            return false;
        }
        record_gold_donated(quantity);
        return true;
    }

    if (static_cast<uint8_t>(set) == 0 || !this->community_center.is_valid_item(set, item)) {
        return false;
    }

    std::vector<std::unique_ptr<Item>> donated;
    if (set == CommunitySet::CROPS) {
        if (!p.can_extract_items_with_min_quality(item, Quality::INCREDIBLE, quantity)) {
            return false;
        }
        donated = p.extract_items_with_min_quality(item, Quality::INCREDIBLE, quantity);
    } else {
        if (!p.can_extract_items(item, quantity)) {
            return false;
        }
        donated = p.extract_items(item, quantity);
    }
    if (donated.empty()) {
        return false;
    }

    const uint16_t donated_quantity = static_cast<uint16_t>(donated.size());
    bool offered = false;
    if (set == CommunitySet::CROPS) {
        offered = this->community_center.offer_crop(item, Quality::INCREDIBLE, donated_quantity);
    } else if (set == CommunitySet::MINERALS) {
        offered = this->community_center.offer_mineral(item, donated_quantity);
    } else if (set == CommunitySet::ANIMALS) {
        offered = this->community_center.offer_animal(item, donated_quantity);
    }

    if (offered && this->community_center.try_complete_if_ready(set)) {
        complete_set(set);
    }
    if (!offered) {
        for (auto& item_to_restore: donated) {
            p.pick_up_item(std::move(item_to_restore));
        }
    }
    return offered;
}

void World::record_enemy_killed(const std::string& kind, uint16_t group_id,
                                uint16_t pair_id, uint16_t room) {
    this->community_center.record_enemy_killed(kind, group_id, pair_id, room);
    if (this->community_center.try_complete_if_ready(CommunitySet::COMBAT)) {
        complete_set(CommunitySet::COMBAT);
    }
}

void World::record_gold_donated(uint16_t amount) {
    if (this->community_center.offer_gold(amount)) {
        if (this->community_center.try_complete_if_ready(CommunitySet::MONEY)) {
            complete_set(CommunitySet::MONEY);
        }
    }
}

bool World::is_market_discount_active() const { return this->market_discount_active; }

bool World::is_double_sell_active() const { return this->double_sell_active; }

void World::activate_market_rewards() {
    this->market_discount_active = true;
    this->double_sell_active = true;
}

std::vector<uint8_t> World::take_completed_sets() {
    return this->community_center.take_completed_sets();
}

bool World::take_victory() {
    if (!this->community_victory) {
        return false;
    }
    this->community_victory = false;
    return true;
}

// ---------------------------------------------------------
// REGALOS
// ---------------------------------------------------------
void World::notify_gift_request(uint8_t sender, uint8_t receiver, ItemID item) {
    this->pending_gifts.push_back({sender, receiver, item});
}

void World::notify_gift_answered(uint8_t sender_id) {
    this->gift_answers.push_back(sender_id);
}

std::vector<GiftOffer> World::take_gift_requests() {
    return std::move(this->pending_gifts);
}

std::vector<uint8_t> World::take_gift_answers() {
    return std::move(this->gift_answers);
}

void World::process_gift_acceptance(uint8_t sender_id, uint8_t receiver_id) {
    auto sender_it = this->players.find(sender_id);
    auto receiver_it = this->players.find(receiver_id);
    if (sender_it == this->players.end() || receiver_it == this->players.end()) {
        return;
    }
    sender_it->second->finalize_gift_reception(*receiver_it->second);
}
