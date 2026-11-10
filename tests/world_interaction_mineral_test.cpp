#include <memory>

#include <gtest/gtest.h>

#include "../common/common_constants.h"
#include "../server/domain/entities/player.h"
#include "../server/domain/inventory/item_types.h"
#include "../server/domain/map/world.h"
#include "../server/domain/minerals/coal_rock.h"
#include "../server/domain/minerals/gold_rock.h"
#include "../server/world_view.h"

TEST(WorldInteractionTest, MoversePicarYLevantarCarbonAutomaticamente) {
    World world(5, 5);

    // Spawneamos al jugador en (1, 1) y un Carbón en (2, 1)
    auto player = std::make_unique<Player>(1, 1, 1);
    const Player* p_ref = player.get();
    world.add_player(1, std::move(player));

    world.set_entity_at(2, 1, std::make_unique<CoalRock>(2, 1));

    // El jugador mira hacia la roca
    world.move_player(1, Direction::RIGHT);

    // Verificamos que tiene el Pico equipado por defecto en el slot 0[cite: 3]
    EXPECT_EQ(p_ref->get_equipped_tool(), ToolType::PICKAXE);

    // Picar la roca
    world.use_tool(1);

    // La roca desapareció, el camino está libre para pisar el Drop
    EXPECT_TRUE(world.can_walk_to(2, 1));

    // El jugador AVANZA hacia el drop.
    // Como implementamos el auto-pickup, al pisar (2,1) el ítem se levanta.
    world.move_player(1, Direction::RIGHT);

    // Verificamos que efectivamente avanzó
    EXPECT_EQ(p_ref->get_x(), 2);
    EXPECT_EQ(p_ref->get_y(), 1);

    // VERIFICACIÓN FINAL: El Inventario
    // Los slots 0 a 3 están ocupados por herramientas
    // El slot 4 debería tener nuestro flamante Orbe de Carbón.
    const Inventory& inv = p_ref->get_inventory();

    ASSERT_FALSE(inv.is_slot_empty(4));
    EXPECT_EQ(inv.peek_item(4)->get_id(), ItemID::COAL_ORB);
}

TEST(WorldInteractionTest, EveryAcceptedHitEmitsAnEventIncludingDestruction) {
    World world(4, 4);
    world.add_player(1, std::make_unique<Player>(1, 0, 1));
    world.set_player_facing(1, Direction::RIGHT);
    world.set_entity_at(1, 1, std::make_unique<GoldRock>(1, 1));
    WorldView view;
    view.sync(world, 1);

    for (int hit = 1; hit <= 4; ++hit) {
        world.use_tool(1);
        const auto events = world.take_resource_hits();
        ASSERT_EQ(events.size(), 1u);
        EXPECT_EQ(events[0].x, 1);
        EXPECT_EQ(events[0].y, 1);
        EXPECT_EQ(events[0].destroyed, hit == 4);
        EXPECT_TRUE(world.take_resource_hits().empty());
        const auto changes = view.sync(world, 1);
        if (hit == 2) {
            // Cracked -> Cracked: still emits an impact despite no visual update.
            EXPECT_TRUE(changes.empty());
        }
    }
    world.use_tool(1);  // A dropped item is not a mineral hit.
    EXPECT_TRUE(world.take_resource_hits().empty());
}

TEST(WorldInteractionTest, WrongToolAndEmptyCellDoNotEmitResourceHits) {
    World world(4, 4);
    auto player = std::make_unique<Player>(1, 0, 1);
    player->select_slot(1);  // Shovel.
    world.add_player(1, std::move(player));
    world.set_player_facing(1, Direction::RIGHT);
    auto gold_rock = std::make_unique<GoldRock>(1, 1);
    
    IEntity* rock_ptr = gold_rock.get(); 
    
    world.set_entity_at(1, 1, std::move(gold_rock));
    
    world.use_tool(1);
    EXPECT_TRUE(world.take_resource_hits().empty());
    
    world.remove_entity(rock_ptr); 
    
    world.use_tool(1);
    EXPECT_TRUE(world.take_resource_hits().empty());
}
