#include "dungeon_floor.h"

#include <algorithm>
#include <cstdlib>
#include <utility>

#include "../enemies/enemy.h"
#include "../enemies/orc.h"
#include "../enemies/vampire.h"
#include "../enemies/vampiress.h"
#include "../entities/i_movable.h"
#include "world.h"

#define kOrcPackMin 3
#define kOrcPackMax 6
#define kOrcPackRadius 2
#define kVampirePairRadius 1
#define kVampirePairMinCells 2

static Direction direction_to(uint16_t x, uint16_t y, uint16_t target_x, uint16_t target_y) {
    const int dx = static_cast<int>(target_x) - static_cast<int>(x);
    const int dy = static_cast<int>(target_y) - static_cast<int>(y);
    if (std::abs(dx) >= std::abs(dy)) {
        return dx >= 0 ? Direction::RIGHT : Direction::LEFT;
    }
    return dy >= 0 ? Direction::DOWN : Direction::UP;
}

DungeonFloor::DungeonFloor(uint8_t room_number, uint16_t width, uint16_t height,
                           uint16_t spawn_x, uint16_t spawn_y):
        Scene(width, height),
        room_number(room_number),
        spawn_x(spawn_x),
        spawn_y(spawn_y),
        enemies(),
        flames(),
        blood_puddles() {}

void DungeonFloor::on_entity_removed(IEntity* entity) {
    enemies.erase(std::remove(enemies.begin(), enemies.end(), entity), enemies.end());
}

uint8_t DungeonFloor::get_room_number() const { return this->room_number; }

SceneId DungeonFloor::get_scene_id() const {
    return static_cast<SceneId>(static_cast<uint8_t>(SceneId::DUNGEON_1) +
                                (this->room_number - 1));
}

uint16_t DungeonFloor::get_spawn_x() const { return this->spawn_x; }

uint16_t DungeonFloor::get_spawn_y() const { return this->spawn_y; }

void DungeonFloor::add_enemy(std::unique_ptr<Enemy> enemy, uint16_t enemy_id) {
    if (!enemy) {
        return;
    }
    const uint16_t x = enemy->get_x();
    const uint16_t y = enemy->get_y();
    if (!in_bounds(x, y)) {
        return;
    }
    enemy->set_id(enemy_id);
    enemy->set_room(this->room_number);
    set_entity_at(x, y, std::move(enemy));
    set_entity_at(x, y, std::move(enemy));
    enemies.push_back(static_cast<Enemy*>(this->grid[y][x].entities.back().get()));
}

void DungeonFloor::spawn_orc_pack(uint16_t x, uint16_t y, uint16_t& next_enemy_id, uint16_t group_id) {
    const int pack_size = kOrcPackMin + std::rand() % (kOrcPackMax - kOrcPackMin + 1);
    std::vector<std::pair<uint16_t, uint16_t>> cells;
    for (int radius = 0; radius <= kOrcPackRadius; ++radius) {
        for (int dy = -radius; dy <= radius; ++dy) {
            for (int dx = -radius; dx <= radius; ++dx) {
                if (std::max(std::abs(dx), std::abs(dy)) != radius) {
                    continue;
                }
                const int cell_x = static_cast<int>(x) + dx;
                const int cell_y = static_cast<int>(y) + dy;
                if (cell_x < 0 || cell_y < 0 || cell_x >= this->width || cell_y >= this->height) {
                    continue;
                }
                const uint16_t new_x = static_cast<uint16_t>(cell_x);
                const uint16_t new_y = static_cast<uint16_t>(cell_y);
                if (can_walk_to(new_x, new_y)) {
                    cells.emplace_back(new_x, new_y);
                }
            }
        }
    }
    const size_t start = cells.empty() ? 0 : static_cast<size_t>(std::rand()) % cells.size();
    for (int i = 0; i < pack_size && static_cast<size_t>(i) < cells.size(); ++i) {
        const auto& [orc_x, orc_y] = cells[(start + i) % cells.size()];
        if (can_walk_to(orc_x, orc_y)) {
            auto orc = std::make_unique<Orc>(orc_x, orc_y);
            orc->set_group_id(group_id);
            add_enemy(std::move(orc), next_enemy_id++);
        }
    }
}

void DungeonFloor::spawn_vampire_pair(uint16_t x, uint16_t y, uint16_t& next_enemy_id, uint16_t pair_id) {
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
            if (can_walk_to(new_x, new_y)) {
                cells.emplace_back(new_x, new_y);
            }
        }
    }
    if (cells.size() < kVampirePairMinCells) {
        return;
    }
    const size_t start = static_cast<size_t>(std::rand()) % cells.size();
    const auto& [first_x, first_y] = cells[start];
    const auto& [second_x, second_y] = cells[(start + 1) % cells.size()];

    auto vampire = std::make_unique<Vampire>(first_x, first_y);
    auto vampiress = std::make_unique<Vampiress>(second_x, second_y);
    vampire->set_pair_id(pair_id);
    vampiress->set_pair_id(pair_id);
    Vampire* vampire_ptr = vampire.get();
    Vampiress* vampiress_ptr = vampiress.get();
    vampire_ptr->set_partner(vampiress_ptr);
    vampiress_ptr->set_partner(vampire_ptr);
    add_enemy(std::move(vampire), next_enemy_id++);
    add_enemy(std::move(vampiress), next_enemy_id++);
}

void DungeonFloor::clear_enemies() {
    const std::vector<Enemy*> all = this->enemies;
    for (Enemy* enemy: all) {
        remove_entity(enemy);
    }
}

void DungeonFloor::tick_enemies(World& world) {
    const std::vector<Enemy*> today = this->enemies;
    for (Enemy* enemy: today) {
        enemy->update(world);
    }
}

bool DungeonFloor::tick_enemies_pursuit(World& world) {
    const std::vector<Enemy*> today = this->enemies;
    bool changed = false;
    for (Enemy* enemy: today) {
        if (enemy->get_health() == 0) {
            continue;
        }
        changed = enemy->pursue(world) || changed;
    }
    return changed;
}

void DungeonFloor::spawn_flame(uint16_t projectile_id, uint16_t x, uint16_t y,
                               uint16_t target_x, uint16_t target_y,
                               uint8_t damage, const std::string& kind) {
    const Direction direction = direction_to(x, y, target_x, target_y);
    this->flames.push_back(
            Flame{projectile_id, kind, x, y, direction, damage, target_x, target_y});
}

void DungeonFloor::clear_flames() { this->flames.clear(); }

bool DungeonFloor::tick_flames(World& world) {
    if (this->flames.empty()) {
        return false;
    }
    const SceneId scene_id = get_scene_id();
    auto it = this->flames.begin();
    while (it != this->flames.end()) {
        Flame& flame = *it;
        if (world.damage_players_at(scene_id, flame.x, flame.y, flame.damage)) {
            it = this->flames.erase(it);
            continue;
        }
        const Direction direction = direction_to(flame.x, flame.y, flame.target_x, flame.target_y);
        const auto [next_x, next_y] = IMovable::next_tile(flame.x, flame.y, direction);
        if ((next_x == flame.x && next_y == flame.y) || !can_walk_to(next_x, next_y)) {
            it = this->flames.erase(it);
            continue;
        }
        flame.x = next_x;
        flame.y = next_y;
        flame.direction = direction;
        if (world.damage_players_at(scene_id, flame.x, flame.y, flame.damage) ||
            (flame.x == flame.target_x && flame.y == flame.target_y)) {
            it = this->flames.erase(it);
            continue;
        }
        ++it;
    }
    return true;
}

void DungeonFloor::add_blood_puddle(uint16_t x, uint16_t y) {
    this->blood_puddles.emplace_back(x, y);
}

bool DungeonFloor::consume_blood_puddle() {
    if (this->blood_puddles.empty()) {
        return false;
    }
    this->blood_puddles.pop_back();
    return true;
}

bool DungeonFloor::has_blood_puddle() const { return !this->blood_puddles.empty(); }

void DungeonFloor::clear_blood_puddle() { this->blood_puddles.clear(); }

void DungeonFloor::on_new_day(World& /*world*/) {
    clear_blood_puddle();
    clear_flames();
    clear_enemies();
}

const std::vector<std::pair<uint16_t, uint16_t>>& DungeonFloor::blood_puddle_list() const {
    return this->blood_puddles;
}

std::vector<ProjectileSnapshot> DungeonFloor::projectile_snapshots() const {
    std::vector<ProjectileSnapshot> out;
    out.reserve(this->flames.size());
    for (const Flame& flame: this->flames) {
        out.push_back(
                ProjectileSnapshot{flame.id, flame.kind, flame.x, flame.y, flame.direction});
    }
    return out;
}
