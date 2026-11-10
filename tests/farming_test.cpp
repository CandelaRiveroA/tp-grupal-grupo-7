#include <gtest/gtest.h>
#include <memory>

#include "../server/domain/map/world.h"
#include "../server/domain/entities/player.h"
#include "../server/domain/crops/crop.h"
#include "../server/domain/crops/strawberry.h"
#include "../server/domain/crops/green_onion.h"
#include "../common/common_constants.h"
#include "../server/domain/inventory/item_types.h"

// ---------------------------------------------------------
// TEST 1: El Suelo (Arar, Regar y Secar)
// ---------------------------------------------------------
TEST(FarmingTest, CicloDeHumedadDelSuelo) {
    World world(5, 5); 
    auto player = std::make_unique<Player>(1, 2, 2);
    Player* p_ref = player.get();
    world.add_player(1, std::move(player));

    // CORRECCIÓN: Solo lo hacemos mirar hacia (3,2) sin moverlo de celda
    world.set_player_facing(1, Direction::RIGHT);
    
    // 1. Equipamos la Pala y aramos (Slot 1)
    p_ref->select_slot(1);
    world.use_tool(1); 
    
    EXPECT_TRUE(world.get_cell(3, 2).is_soil_tilled());

    // 2. Equipamos la Regadera y regamos (Slot 2)
    p_ref->select_slot(2);
    world.use_tool(1);

    EXPECT_TRUE(world.get_cell(3, 2).is_soil_watered());

    // 3. Simulamos el fin de un día
    world.process_end_of_day_floor(); 
    
    EXPECT_TRUE(world.get_cell(3, 2).is_soil_tilled());
    EXPECT_FALSE(world.get_cell(3, 2).is_soil_watered());
}

// ---------------------------------------------------------
// TEST 2: Crecimiento y Cosecha Simple (Cebolla de Verdeo)
// ---------------------------------------------------------
TEST(FarmingTest, CebollaDeVerdeoCreceCosechaYDesaparece) {
    World world(5, 5);
    auto player = std::make_unique<Player>(1, 2, 2);
    Player* p_ref = player.get();
    world.add_player(1, std::move(player));
    
    world.set_player_facing(1, Direction::RIGHT);

    // Preparamos la tierra usando la pala (así respetamos el encapsulamiento)
    p_ref->select_slot(1);
    world.use_tool(1);
    
    // Plantamos la semilla artificialmente
    world.set_entity_at(3, 2, std::make_unique<GreenOnion>(3, 2));
    
    for (int day = 0; day < 2; ++day) {
        p_ref->select_slot(2);
        world.use_tool(1);
        world.use_tool(1);
        world.use_tool(1);
        world.process_end_of_day_floor(); 
    }

    // Cosechamos
    p_ref->select_slot(8); 
    world.use_tool(1);

    // La cebolla desaparece al cosecharse, la celda queda libre
    EXPECT_TRUE(world.can_walk_to(3, 2)); 
    
    // Ahora que está libre, avanzamos para accionar el Auto-pickup
    world.move_player(1, Direction::RIGHT);
    
    bool found = false;
    for (int i = 0; i < 9; ++i) {
        if (!p_ref->get_inventory().is_slot_empty(i) && 
             p_ref->get_inventory().peek_item(i)->get_id() == ItemID::GREEN_ONION) {
            found = true;
            break;
        }
    }
    EXPECT_TRUE(found) << "La cebolla no llegó al inventario.";
}

// ---------------------------------------------------------
// TEST 3: Crecimiento y Rebrote de Frutilla
// ---------------------------------------------------------
TEST(FarmingTest, FrutillaRebrotaDespuesDeCosechar) {
    World world(5, 5);
    auto player = std::make_unique<Player>(1, 2, 2);
    Player* p_ref = player.get();
    world.add_player(1, std::move(player));
    
    world.set_player_facing(1, Direction::RIGHT);

    p_ref->select_slot(1);
    world.use_tool(1);
    world.set_entity_at(3, 2, std::make_unique<Strawberry>(3, 2));
    
    for (int day = 0; day < 4; ++day) {
        p_ref->select_slot(2);
        world.use_tool(1);
        world.use_tool(1);
        world.process_end_of_day_floor();
    }

    // Primera Cosecha
    p_ref->select_slot(8); 
    world.use_tool(1); // Planta dropea la frutilla, pero NO se destruye
    
    // La planta de frutilla sigue bloqueando
    EXPECT_TRUE(world.can_walk_to(3, 2)); 
    
    // CORRECCIÓN: Como no podemos caminar para accionar el auto-pickup,
    // usamos la mano nuevamente hacia adelante para levantar el ítem del piso.
    world.use_tool(1); 
    
    // Segundo ciclo (Rebrote)
    for (int day = 0; day < 2; ++day) {
        p_ref->select_slot(2);
        world.use_tool(1);
        world.use_tool(1);
        world.process_end_of_day_floor();
    }

    // Segunda cosecha
    p_ref->select_slot(8);
    world.use_tool(1); // Cosechamos
    world.use_tool(1); // Recogemos el ítem
    
    int strawberry_count = 0;
    for (int i = 0; i < 9; ++i) {
        if (!p_ref->get_inventory().is_slot_empty(i) && 
             p_ref->get_inventory().peek_item(i)->get_id() == ItemID::STRAWBERRY) {
            strawberry_count += p_ref->get_inventory().get_amount(i);
        }
    }
    
    EXPECT_EQ(strawberry_count, 2) << "El rebrote no produjo la segunda frutilla.";
}