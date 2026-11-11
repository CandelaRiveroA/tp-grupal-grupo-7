#include <gtest/gtest.h>
#include <memory>
#include "../server/domain/map/world.h"
#include "../server/domain/entities/player.h"
#include "../server/domain/minerals/coal_rock.h"
#include "../server/domain/minerals/gold_rock.h"
#include "../server/domain/minerals/rock.h"
#include "../common/common_constants.h"

// ---------------------------------------------------------
// TEST 1: El carbón se rompe de un solo golpe
// ---------------------------------------------------------
TEST(MineralTest, CarbonSeRompeConUnGolpe) {
    World world(3, 3);
    auto p = std::make_unique<Player>(1, 0, 1);
    world.add_player(1, std::move(p));

    world.set_entity_at(1, 1, std::make_unique<CoalRock>(1, 1));
    
    // Al principio, la celda está bloqueada por la roca
    EXPECT_FALSE(world.can_walk_to(1, 1));

    // El jugador mira a la derecha (hacia la roca en 1,1) y usa el pico (Slot 0)[cite: 3]
    world.move_player(1, Direction::RIGHT); 
    world.use_tool(1); 

    // Como requiere 1 golpe, la roca debió desaparecer y dejar el ítem pisable
    EXPECT_TRUE(world.can_walk_to(1, 1));
}

// ---------------------------------------------------------
// TEST 2: El Oro requiere exactamente 4 golpes
// ---------------------------------------------------------
TEST(MineralTest, OroRequiereCuatroGolpesParaRomperse) {
    World world(3, 3);
    auto p = std::make_unique<Player>(1, 0, 1);
    world.add_player(1, std::move(p));

    world.set_entity_at(1, 1, std::make_unique<GoldRock>(1, 1));
    world.move_player(1, Direction::RIGHT); // Mirar hacia el oro

    // Damos 3 golpes
    world.use_tool(1);
    world.use_tool(1);
    world.use_tool(1);

    // Todavía no se rompió, debe seguir bloqueando el paso
    EXPECT_FALSE(world.can_walk_to(1, 1));

    // Damos el 4to golpe 
    world.use_tool(1);

    //Ahora se rompió y el ítem es pisable
    EXPECT_TRUE(world.can_walk_to(1, 1));
}

// ---------------------------------------------------------
// TEST 3: La roca de relleno se rompe de un golpe y no deja nada
// ---------------------------------------------------------
TEST(MineralTest, RocaSeRompeDeUnGolpeSinDejarDrop) {
    World world(3, 3);
    auto p = std::make_unique<Player>(1, 0, 1);
    world.add_player(1, std::move(p));

    world.set_entity_at(1, 1, std::make_unique<Rock>(1, 1));
    EXPECT_FALSE(world.can_walk_to(1, 1));

    world.move_player(1, Direction::RIGHT);
    world.use_tool(1);

    EXPECT_TRUE(world.can_walk_to(1, 1));
    // Nada en el piso: ni drop ni resto de la roca.
    EXPECT_TRUE(world.get_entity_at(1, 1).empty());
}
