#include <memory>
#include <set>
#include <string>

#include <gtest/gtest.h>

#include "../server/domain/enemies/enemy_controller.h"
#include "../server/domain/enemies/skeleton.h"
#include "../server/domain/map/portal.h"
#include "../server/domain/map/world.h"
#include "../server/domain/minerals/mine_controller.h"
#include "../server/domain/minerals/rock.h"

namespace {

class CaveScenario {
public:
    World world;

    CaveScenario(): world(20, 20) {
        world.set_active_scene("dungeon_1");
    }

    void block_all() {
        Scene& scene = world.get_scene("dungeon_1");
        for (uint16_t y = 0; y < 20; ++y) {
            for (uint16_t x = 0; x < 20; ++x) scene.set_wall(x, y, true);
        }
    }

    void open_rectangle(uint16_t left, uint16_t top, uint16_t right, uint16_t bottom) {
        Scene& scene = world.get_scene("dungeon_1");
        for (uint16_t y = top; y <= bottom; ++y) {
            for (uint16_t x = left; x <= right; ++x) scene.set_wall(x, y, false);
        }
    }

    std::set<uint16_t> enemy_ids() const {
        std::set<uint16_t> ids;
        for (uint16_t y = 0; y < 20; ++y) {
            for (uint16_t x = 0; x < 20; ++x) {
                for (const IEntity* entity: world.get_entity_at(x, y)) {
                    if (const auto enemy = entity->enemy_snapshot()) ids.insert(enemy->id);
                }
            }
        }
        return ids;
    }

    std::size_t minerals() const {
        std::size_t count = 0;
        for (uint16_t y = 0; y < 20; ++y) {
            for (uint16_t x = 0; x < 20; ++x) {
                for (const IEntity* entity: world.get_entity_at(x, y)) {
                    const auto cell = entity->snapshot();
                    if (cell && cell->kind == CellSnapshot::Kind::Resource) ++count;
                }
            }
        }
        return count;
    }

    void verify_terrain() const {
        const Scene& scene = world.get_scene("dungeon_1");
        for (uint16_t y = 0; y < 20; ++y) {
            for (uint16_t x = 0; x < 20; ++x) {
                for (const IEntity* entity: scene.get_entity_at(x, y)) {
                    const auto resource = entity->snapshot();
                    if (entity->enemy_snapshot() ||
                        (resource && resource->kind == CellSnapshot::Kind::Resource)) {
                        EXPECT_FALSE(scene.get_cell(x, y).is_wall);
                        EXPECT_EQ(scene.get_entity_at(x, y).size(), 1u);
                    }
                }
            }
        }
    }
};

TEST(CaveSpawnTest, MineralesYEnemigosRespetanElAreaCaminableIrregular) {
    CaveScenario cave;
    cave.block_all();
    cave.open_rectangle(3, 3, 16, 16);
    for (uint16_t y = 5; y < 15; ++y) {
        cave.world.get_scene("dungeon_1").set_wall(5, y, true);
    }
    cave.world.set_entity_at(3, 3, std::make_unique<Portal>(3, 3, "farm", 1, 1));
    for (uint32_t seed = 1; seed <= 10; ++seed) {
        cave.world.clear_enemies();
        MineController(1, seed).populate(cave.world, 10, 10);
        EnemyController(1, seed).populate(cave.world, 10, 10);
        EXPECT_GT(cave.minerals(), 0u);
        EXPECT_FALSE(cave.enemy_ids().empty());
        cave.verify_terrain();
        EXPECT_TRUE(cave.world.get_entity_at(10, 10).empty());
        EXPECT_EQ(cave.world.get_entity_at(3, 3).size(), 1u);
    }
}

TEST(CaveSpawnTest, SalaCompletamenteBloqueadaNoGeneraEntidades) {
    CaveScenario cave;
    cave.block_all();
    MineController(1, 42).populate(cave.world, 10, 10);
    EnemyController(1, 42).populate(cave.world, 10, 10);
    EXPECT_EQ(cave.minerals(), 0u);
    EXPECT_TRUE(cave.enemy_ids().empty());
}

TEST(CaveSpawnTest, SalaPequenaConEspacioGeneraAlMenosUnEnemigo) {
    CaveScenario cave;
    cave.block_all();
    cave.open_rectangle(8, 8, 12, 12);
    MineController(1, 42).populate(cave.world, 10, 10);
    EnemyController(1, 42).populate(cave.world, 10, 10);
    EXPECT_GT(cave.minerals(), 0u);
    EXPECT_FALSE(cave.enemy_ids().empty());
    cave.verify_terrain();
}

TEST(CaveSpawnTest, ManadasYParejasEvitanParedesPortalesYLaEntrada) {
    CaveScenario cave;
    cave.block_all();
    cave.open_rectangle(8, 8, 12, 12);
    cave.world.set_entity_at(8, 8, std::make_unique<Portal>(8, 8, "farm", 1, 1));
    cave.world.set_entity_at(9, 8, std::make_unique<Rock>(9, 8));
    cave.world.spawn_orc_pack(9, 9);
    cave.world.spawn_vampire_pair(11, 11);
    EXPECT_FALSE(cave.enemy_ids().empty());
    cave.verify_terrain();
    EXPECT_TRUE(cave.world.get_entity_at(10, 10).empty());
    EXPECT_EQ(cave.world.get_entity_at(8, 8).size(), 1u);
}

TEST(CaveSpawnTest, AddEnemyRechazaUnaParedYUnaCeldaYaOcupada) {
    CaveScenario cave;
    cave.world.get_scene("dungeon_1").set_wall(3, 3, true);
    cave.world.add_enemy(std::make_unique<Skeleton>(3, 3));
    cave.world.set_entity_at(4, 4, std::make_unique<Rock>(4, 4));
    cave.world.add_enemy(std::make_unique<Skeleton>(4, 4));
    EXPECT_TRUE(cave.enemy_ids().empty());
    EXPECT_TRUE(cave.world.get_entity_at(3, 3).empty());
    EXPECT_EQ(cave.world.get_entity_at(4, 4).size(), 1u);
}

TEST(CaveSpawnTest, CadaAmanecerReponeEnemigosYMineralesDespuesDeLimpiar) {
    CaveScenario cave;
    cave.block_all();
    cave.open_rectangle(3, 3, 16, 16);
    cave.world.add_player(1, std::make_unique<Player>(1, 1, 1, "farmhouse"));
    MineController(1, 42).populate(cave.world, 10, 10);
    EnemyController(1, 42).populate(cave.world, 10, 10);
    auto previous_ids = cave.enemy_ids();
    EXPECT_FALSE(previous_ids.empty());
    for (int day = 0; day < 3; ++day) {
        cave.world.clear_enemies();  // Simula haber vaciado la sala.
        cave.world.end_day();
        EXPECT_TRUE(cave.world.take_day_ended());
        cave.world.answer_wake(1);
        EXPECT_TRUE(cave.world.take_day_started());
        const auto current_ids = cave.enemy_ids();
        EXPECT_FALSE(current_ids.empty());
        EXPECT_GT(cave.minerals(), 0u);
        for (uint16_t id: current_ids) EXPECT_EQ(previous_ids.count(id), 0u);
        previous_ids = current_ids;
        cave.verify_terrain();
        EXPECT_TRUE(cave.world.get_entity_at(10, 10).empty());
        EXPECT_EQ(cave.world.get_active_scene(), "dungeon_1");
    }
}

}  // namespace
