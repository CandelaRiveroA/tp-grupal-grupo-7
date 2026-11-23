#include <memory>

#include <gtest/gtest.h>

#include "../server/domain/animals/chicken.h"
#include "../server/domain/animals/cow.h"
#include "../server/domain/entities/player.h"
#include "../server/domain/map/world.h"

TEST(AnimalsTest, CowIsReadyToMilkAfterTwoDays) {
    World world(5, 5);
    world.set_active_scene(SceneId::FARM);
    world.add_player(1, std::make_unique<Player>(1, 1, 1, SceneId::FARM));
    world.add_animal(std::make_unique<Cow>(2, 1));

    world.tick_day();
    world.take_item(1);
    EXPECT_TRUE(world.take_pickups().empty());

    world.tick_day();
    world.take_item(1);
    const std::vector<PlayerPickup> pickups = world.take_pickups();
    ASSERT_EQ(pickups.size(), 1u);
    EXPECT_EQ(pickups[0].item, ItemID::MILK);
}

// La gallina también queda en la lista única de animales: si respondiera "mano
// consumida" comería la acción de ordeño de la vaca de al lado.
TEST(AnimalsTest, LaGallinaAdyacenteNoSeLlevaLaAccionDeLaMano) {
    World world(5, 5);
    world.set_active_scene(SceneId::FARM);
    world.add_player(1, std::make_unique<Player>(1, 1, 1, SceneId::FARM));
    world.add_animal(std::make_unique<Chicken>(2, 1));
    world.add_animal(std::make_unique<Cow>(1, 2));

    world.tick_day();
    world.tick_day();
    world.take_item(1);

    const std::vector<PlayerPickup> pickups = world.take_pickups();
    ASSERT_EQ(pickups.size(), 1u);
    EXPECT_EQ(pickups[0].item, ItemID::MILK);
}

TEST(AnimalsTest, ChickenLaysAnEggEveryDayOnItsCell) {
    World world(5, 5);
    world.set_active_scene(SceneId::FARM);
    world.add_animal(std::make_unique<Chicken>(3, 3));

    world.tick_day();
    EXPECT_EQ(world.get_farm().get_entity_at(3, 3).size(), 2u);

    world.tick_day();
    EXPECT_EQ(world.get_farm().get_entity_at(3, 3).size(), 3u);
}

// ---------------------------------------------------------
// Movimiento aleatorio
// ---------------------------------------------------------
TEST(AnimalsTest, ElAnimalSeMueveAUnaCeldaLibre) {
    World world(5, 5);
    world.set_active_scene(SceneId::FARM);
    world.add_animal(std::make_unique<Chicken>(1, 1));
    IEntity* chicken = world.get_farm().get_entity_at(1, 1).at(0);

    world.move_entity(chicken, 2, 1);

    EXPECT_EQ(chicken->get_x(), 2u);
    EXPECT_EQ(world.get_farm().get_entity_at(1, 1).size(), 0u);
    EXPECT_EQ(world.get_farm().get_entity_at(2, 1).size(), 1u);
}

TEST(AnimalsTest, ElAnimalNoEntraEnUnaCeldaOcupadaPorUnaVaca) {
    World world(5, 5);
    world.set_active_scene(SceneId::FARM);
    world.add_animal(std::make_unique<Chicken>(1, 1));
    world.add_animal(std::make_unique<Cow>(2, 1));
    IEntity* chicken = world.get_farm().get_entity_at(1, 1).at(0);

    world.move_entity(chicken, 2, 1);

    EXPECT_EQ(chicken->get_x(), 1u);
    EXPECT_EQ(world.get_farm().get_entity_at(1, 1).size(), 1u);
}

TEST(AnimalsTest, TickAnimalesMantieneAlAnimalDentroDelMapa) {
    World world(5, 5);
    world.set_active_scene(SceneId::FARM);
    world.add_animal(std::make_unique<Chicken>(0, 0));
    const IEntity* chicken = world.get_farm().get_entity_at(0, 0).at(0);

    for (int step = 0; step < 50; ++step) {
        world.tick_animals();
    }

    EXPECT_LT(chicken->get_x(), 5u);
    EXPECT_LT(chicken->get_y(), 5u);
}
