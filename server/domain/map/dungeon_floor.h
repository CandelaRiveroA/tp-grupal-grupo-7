#ifndef DUNGEON_FLOOR_H
#define DUNGEON_FLOOR_H

#include <cstdint>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "scene.h"

class Enemy;
class World;

class DungeonFloor: public Scene {
private:
    uint8_t room_number;
    uint16_t spawn_x;
    uint16_t spawn_y;

    std::vector<Enemy*> enemies;
    std::vector<Flame> flames;
    std::vector<std::pair<uint16_t, uint16_t>> blood_puddles;

protected:
    void on_entity_removed(IEntity* entity) override;

public:
    DungeonFloor(uint8_t room_number, uint16_t width, uint16_t height,
                 uint16_t spawn_x, uint16_t spawn_y);
    ~DungeonFloor() override = default;

    uint8_t get_room_number() const;
    uint16_t get_spawn_x() const;
    uint16_t get_spawn_y() const;
    SceneId get_scene_id() const;

    // --- Enemigos ---
    void add_enemy(std::unique_ptr<Enemy> enemy, uint16_t enemy_id);
   void spawn_orc_pack(uint16_t x, uint16_t y, uint16_t& next_enemy_id, uint16_t group_id);
void spawn_vampire_pair(uint16_t x, uint16_t y, uint16_t& next_enemy_id, uint16_t pair_id);
    void clear_enemies();
    void tick_enemies(World& world);
    bool tick_enemies_pursuit(World& world);

    // --- Llamas (proyectiles) ---
    void spawn_flame(uint16_t projectile_id, uint16_t x, uint16_t y,
                     uint16_t target_x, uint16_t target_y,
                     uint8_t damage, const std::string& kind);
    void clear_flames();
    bool tick_flames(World& world);

    // --- Charcos de sangre ---
    void add_blood_puddle(uint16_t x, uint16_t y);
    bool consume_blood_puddle();
    bool has_blood_puddle() const;
    void clear_blood_puddle();

    // --- Ciclo de vida y WorldView ---
    void on_new_day(World& world) override;
    const std::vector<std::pair<uint16_t, uint16_t>>& blood_puddle_list() const override;
    std::vector<ProjectileSnapshot> projectile_snapshots() const override;
};

#endif  // DUNGEON_FLOOR_H
