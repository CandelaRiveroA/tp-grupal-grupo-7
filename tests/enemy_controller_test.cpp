#include "../server/domain/enemies/enemy_controller.h"

#include <memory>

#include <gtest/gtest.h>

#include "../server/domain/enemies/enemy.h"
#include "../server/domain/enemies/skeleton.h"
#include "../server/domain/enemies/skull.h"
#include "../server/domain/enemies/vampire.h"
#include "../server/domain/enemies/vampiress.h"
#include "../server/domain/map/world.h"
#include "../server/domain/minerals/rock.h"

namespace {

// Cuenta las entidades del tipo T en todo el mapa.
template <typename T>
size_t count_type(const World& world) {
    size_t count = 0;
    for (uint16_t y = 0; y < world.get_height(); ++y) {
        for (uint16_t x = 0; x < world.get_width(); ++x) {
            for (const IEntity* entity: world.get_entity_at(x, y)) {
                if (dynamic_cast<const T*>(entity) != nullptr) {
                    ++count;
                }
            }
        }
    }
    return count;
}

}  // namespace

TEST(EnemyControllerTest, GeneraEnemigosSinPisarElSpawn) {
    World world(30, 20);
    world.set_active_scene("dungeon_1");
    EnemyController enemies(1, 42);
    enemies.populate(world, 15, 10);

    EXPECT_GT(count_type<Enemy>(world), 0u);
    EXPECT_TRUE(world.get_entity_at(15, 10).empty());
}

TEST(EnemyControllerTest, UnSoloSkullPorSala) {
    for (const uint8_t sala: {1, 5, 10}) {
        World world(30, 20);
    world.set_active_scene("dungeon_1");
        EnemyController enemies(sala, 42);
        enemies.populate(world, 15, 10);

        EXPECT_EQ(count_type<Skull>(world), 1u) << "sala " << int(sala);
    }
}

TEST(EnemyControllerTest, DificultadIncremental) {
    World early(30, 20);
    early.set_active_scene("dungeon_1");
    EnemyController(1, 42).populate(early, 15, 10);
    World late(30, 20);
    late.set_active_scene("dungeon_1");
    EnemyController(10, 42).populate(late, 15, 10);

    // Sala 1: esqueletos. Sala 10: vampiros.
    EXPECT_GT(count_type<Skeleton>(early), count_type<Skeleton>(late));
    EXPECT_GT(count_type<Vampire>(late) + count_type<Vampiress>(late),
              count_type<Vampire>(early) + count_type<Vampiress>(early));
}

TEST(EnemyControllerTest, RegeneraSinAcumularNiBorrarLosMinerales) {
    // Contexto de la mazmorra: solo minerales y enemigos.
    World world(30, 20);
    world.set_active_scene("dungeon_1");
    world.set_entity_at(0, 0, std::make_unique<Rock>(0, 0));
    world.set_entity_at(1, 0, std::make_unique<Rock>(1, 0));

    EnemyController enemies(1, 7);
    enemies.populate(world, 15, 10);
    // Una llama en vuelo de la generación vieja.
    world.spawn_flame(5, 5, 5, 8, 5, "skull_fire");
    enemies.populate(world, 15, 10);

    // La segunda pasada limpia la generación vieja: un solo Skull.
    EXPECT_EQ(count_type<Skull>(world), 1u);
    // Y también las llamas que quedaron en el aire.
    EXPECT_TRUE(world.projectile_snapshots().empty());
    // Los minerales no son enemigos: quedan donde estaban.
    EXPECT_EQ(world.get_entity_at(0, 0).size(), 1u);
    EXPECT_EQ(world.get_entity_at(1, 0).size(), 1u);
    // Los enemigos de la generación vieja ya no están en la lista de ticks.
    world.tick_enemies();
}
