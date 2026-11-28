#include <memory>
#include <string>
#include <vector>

#include <gtest/gtest.h>

#include "../server/commands/chat_command.h"
#include "../server/domain/animals/chicken.h"
#include "../server/domain/animals/cow.h"

namespace {

std::string run_cheat(ChatRouter& router, World& world, const std::string& command) {
    const auto replies = router.route(command, 1, world, {{1, "Ana"}});
    EXPECT_EQ(replies.size(), 1u);
    EXPECT_FALSE(replies[0].broadcast);
    EXPECT_EQ(replies[0].target, 1);
    return std::get<ServerChat>(replies[0].message).text;
}

std::size_t eggs_at(const World& world, uint16_t x, uint16_t y) {
    std::size_t count = 0;
    for (const IEntity* entity: world.get_scene("barn").get_entity_at(x, y)) {
        const auto snapshot = entity->snapshot();
        if (snapshot && snapshot->kind == CellSnapshot::Kind::Dropped && snapshot->item == ItemID::EGG) {
            ++count;
        }
    }
    return count;
}

}  // namespace

TEST(CareCheatsTest, InfoListaTodosLosComandosRegistradosIncluidosLosNuevos) {
    World world(10, 10);
    ChatRouter router;
    const auto text = run_cheat(router, world, "/info");
    for (const char* command: {"/info", "/cuidar_animales", "/cuidar_cultivos", "/vida",
            "/energia", "/oro", "/terminar_dia", "/tiempo", "/tp", "/tp_info",
            "/completar_set", "/completar_todo", "/consultar_catalogo", "/comprar", "/vender"}) {
        EXPECT_NE(text.find(command), std::string::npos);
    }
    EXPECT_EQ(text.find("/donar"), std::string::npos);
}

TEST(CareCheatsTest, AnimalesProducenEnElGraneroAunqueElJugadorEsteEnOtroMapa) {
    World world(10, 10);
    world.add_player(1, std::make_unique<Player>(1, 1, 1, "town"));
    auto first = std::make_unique<Cow>(3, 3);
    Cow* cow = first.get();
    world.add_animal(std::move(first));
    auto second = std::make_unique<Cow>(4, 3);
    Cow* other = second.get();
    world.add_animal(std::move(second));
    world.add_animal(std::make_unique<Chicken>(3, 4));
    world.add_animal(std::make_unique<Chicken>(4, 4));
    EXPECT_FALSE(cow->animal_snapshot()->ready_to_milk);
    const uint8_t day = world.get_day();
    world.set_active_scene("town");
    ChatRouter router;
    EXPECT_EQ(run_cheat(router, world, "/cuidar_animales"), "animales con produccion lista: 4");
    EXPECT_TRUE(cow->animal_snapshot()->ready_to_milk);
    EXPECT_TRUE(other->animal_snapshot()->ready_to_milk);
    EXPECT_EQ(eggs_at(world, 3, 4), 1u);
    EXPECT_EQ(eggs_at(world, 4, 4), 1u);
    EXPECT_TRUE(world.get_scene("town").get_entity_at(3, 4).empty());
    EXPECT_EQ(world.get_active_scene(), "town");
    EXPECT_EQ(world.get_day(), day);
    cow->interact(world.get_player(1), ToolType::HAND, world);
    EXPECT_FALSE(cow->animal_snapshot()->ready_to_milk);
    EXPECT_TRUE(world.get_player(1).can_extract_items(ItemID::MILK, 1));
    run_cheat(router, world, "/cuidar_animales");
    EXPECT_TRUE(cow->animal_snapshot()->ready_to_milk);
    EXPECT_EQ(eggs_at(world, 3, 4), 2u);
}

TEST(CareCheatsTest, TodosLosCultivosPlantadosMaduranSinAvanzarDiaNiRecolectarlos) {
    World world(10, 10);
    world.add_player(1, std::make_unique<Player>(1, 1, 1, "farm"));
    auto& player = world.get_player(1);
    for (uint8_t i = 0; i < 4; ++i) {
        world.get_farm().till_soil(3 + i, 4);
        player.select_slot(4 + i);
        ASSERT_TRUE(world.get_farm().try_plant_seed(player, 3 + i, 4));
    }
    world.transfer_player(1, "town", 1, 1);
    world.set_active_scene("town");
    const uint8_t day = world.get_day();
    ChatRouter router;
    EXPECT_EQ(run_cheat(router, world, "/cuidar_cultivos"), "cultivos listos para cosechar: 4");
    std::vector<Quality> qualities;
    for (uint16_t x = 3; x < 7; ++x) {
        const auto entities = world.get_scene("farm").get_entity_at(x, 4);
        ASSERT_EQ(entities.size(), 1u);
        const auto crop = entities[0]->crop_snapshot();
        ASSERT_TRUE(crop.has_value());
        EXPECT_TRUE(entities[0]->pickup_on_take());
        EXPECT_EQ(crop->stage, 6);
        qualities.push_back(crop->quality);
    }
    run_cheat(router, world, "/cuidar_cultivos");
    for (uint16_t x = 3; x < 7; ++x) {
        EXPECT_EQ(world.get_scene("farm").get_entity_at(x, 4)[0]->crop_snapshot()->quality,
                  qualities[x - 3]);
    }
    EXPECT_EQ(world.get_day(), day);
    EXPECT_EQ(world.get_active_scene(), "town");
    EXPECT_TRUE(world.take_pickups().empty());
}

TEST(CareCheatsTest, ArgumentosInvalidosNoGeneranProduccionNiMaduranCultivos) {
    World world(10, 10);
    world.add_player(1, std::make_unique<Player>(1, 1, 1, "farm"));
    auto& player = world.get_player(1);
    world.get_farm().till_soil(6, 6);
    player.select_slot(4);
    ASSERT_TRUE(world.get_farm().try_plant_seed(player, 6, 6));
    world.add_animal(std::make_unique<Chicken>(3, 4));
    ChatRouter router;
    EXPECT_EQ(run_cheat(router, world, "/info extra"), "uso: /info");
    EXPECT_EQ(run_cheat(router, world, "/cuidar_animales extra"), "uso: /cuidar_animales");
    EXPECT_EQ(run_cheat(router, world, "/cuidar_cultivos extra"), "uso: /cuidar_cultivos");
    EXPECT_EQ(eggs_at(world, 3, 4), 0u);
    EXPECT_FALSE(world.get_scene("farm").get_entity_at(6, 6)[0]->crop_snapshot()->ready);
}
