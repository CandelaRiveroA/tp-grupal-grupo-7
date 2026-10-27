#include <gtest/gtest.h>
#include <memory>
#include "../server/domain/map/world.h"
#include "../server/domain/entities/player.h"
#include "../server/domain/minerals/coal_rock.h"
#include "../common/common_constants.h"
#include "../server/domain/inventory/item_types.h"

TEST(WorldInteractionTest, MoversePicarYLevantarCarbonAutomaticamente) {
    World world(5, 5); 

    // Spawneamos al jugador en (1, 1) y un Carbón en (2, 1)
    auto player = std::make_unique<Player>(1, 1, 1);
    Player* p_ref = player.get();
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