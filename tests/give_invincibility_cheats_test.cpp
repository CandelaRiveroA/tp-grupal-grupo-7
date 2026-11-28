#include <memory>
#include <set>
#include <sstream>
#include <string>

#include <gtest/gtest.h>

#include "../server/commands/chat_command.h"
#include "../server/world_view.h"

namespace {

std::string give_command(ChatRouter& router, World& world, const std::string& command, uint8_t id = 1) {
    const auto replies = router.route(command, id, world, {{1, "Ana"}, {2, "Beto"}});
    EXPECT_EQ(replies.size(), 1u);
    EXPECT_FALSE(replies[0].broadcast);
    EXPECT_EQ(replies[0].target, id);
    return std::get<ServerChat>(replies[0].message).text;
}

void add_empty_player(World& world, uint8_t id = 1, uint8_t capacity = 9) {
    world.add_player(id, std::make_unique<Player>(id, 1, 1, "farm"));
    world.get_player(id).get_inventory() = Inventory(capacity);
}

}  // namespace

TEST(GiveInvincibilityCheatsTest, DarEntregaCantidadPorDefectoYCalidadParaCompletarLotes) {
    World world(10, 10);
    add_empty_player(world);
    ChatRouter router;
    EXPECT_EQ(give_command(router, world, "/dar frutilla perfecta"), "objetos agregados: 1/1");
    EXPECT_EQ(give_command(router, world, "/dar frutilla 4 3"), "objetos agregados: 3/3");
    const auto& inventory = world.get_player(1).get_inventory();
    ASSERT_NE(inventory.peek_item(0), nullptr);
    EXPECT_EQ(inventory.get_amount(0), 1);
    EXPECT_EQ(inventory.peek_item(0)->get_quality(), Quality::PERFECT);
    EXPECT_EQ(inventory.get_amount(1), 3);
    EXPECT_EQ(inventory.peek_item(1)->get_quality(), Quality::INCREDIBLE);
    EXPECT_TRUE(world.get_player(1).can_extract_items_with_min_quality(ItemID::STRAWBERRY, Quality::INCREDIBLE, 4));
    auto snapshot = WorldView{}.sync(world, 1);
    bool found = false;
    for (const auto& message: snapshot) {
        if (const auto* slot = std::get_if<ServerInventorySlot>(&message)) {
            if (slot->slot == 0) {
                EXPECT_EQ(slot->content_id, "strawberry");
                EXPECT_EQ(slot->quality, 5);
                found = true;
            }
        }
    }
    EXPECT_TRUE(found);
}

TEST(GiveInvincibilityCheatsTest, DarConservaLoQueEntraYAvisaInventarioLlenoSoloAlEmisor) {
    World world(10, 10);
    add_empty_player(world, 1, 1);
    add_empty_player(world, 2);
    ChatRouter router;
    EXPECT_EQ(give_command(router, world, "/dar huevo base 49"), "objetos agregados: 49/49");
    EXPECT_EQ(give_command(router, world, "/dar huevo base 5"), "objetos agregados: 1/5 (inventario lleno)");
    EXPECT_EQ(world.get_player(1).get_inventory().get_amount(0), 50);
    EXPECT_EQ(world.take_inventory_full(), (std::set<uint8_t>{1}));
    EXPECT_TRUE(world.get_player(2).get_inventory().is_slot_empty(0));
    EXPECT_EQ(give_command(router, world, "/dar carbon 1"), "objetos agregados: 0/1 (inventario lleno)");
    EXPECT_EQ(world.get_player(1).get_inventory().peek_item(0)->get_id(), ItemID::EGG);
}

TEST(GiveInvincibilityCheatsTest, DarRechazaParametrosInvalidosAntesDeModificarElInventario) {
    World world(10, 10);
    add_empty_player(world);
    ChatRouter router;
    for (const char* command: {"/dar", "/dar huevo", "/dar desconocido base", "/dar huevo 0",
            "/dar huevo 6", "/dar huevo base 0", "/dar huevo base -1", "/dar huevo base 65536",
            "/dar huevo base 2x", "/dar huevo base 999999999999999999999",
            "/dar huevo base 1 extra", "/dar pico perfecta", "/dar cofre 3"}) {
        give_command(router, world, command);
        EXPECT_TRUE(world.get_player(1).get_inventory().is_slot_empty(0));
        EXPECT_TRUE(world.take_inventory_full().empty());
    }
}

TEST(GiveInvincibilityCheatsTest, DarInfoListaSoloObjetosQueSeCreanConTipoYSpriteValidos) {
    World world(10, 10);
    add_empty_player(world);
    ChatRouter router;
    const auto text = give_command(router, world, "/dar_info");
    const std::string prefix = "Objetos disponibles: ";
    const auto end = text.find(". Calidad:");
    ASSERT_NE(end, std::string::npos);
    std::istringstream stream(text.substr(prefix.size(), end - prefix.size()));
    std::string name;
    unsigned count = 0;
    while (std::getline(stream, name, ',')) {
        name.erase(0, name.find_first_not_of(' '));
        world.get_player(1).get_inventory() = Inventory(9);
        EXPECT_EQ(give_command(router, world, "/dar " + name + " base"), "objetos agregados: 1/1");
        const Item* item = world.get_player(1).get_inventory().peek_item(0);
        ASSERT_NE(item, nullptr);
        EXPECT_FALSE(content_id_for(*item).empty());
        if (name == "pico" || name == "pala" || name == "regadera" || name == "espada") {
            EXPECT_FALSE(item->can_be_traded());
        }
        if (name == "cofre") {
            EXPECT_EQ(item->get_id(), ItemID::CHEST);
        }
        ++count;
    }
    EXPECT_GT(count, 20u);
    EXPECT_EQ(give_command(router, world, "/dar_info extra"), "uso: /dar_info");
    const auto info = give_command(router, world, "/info");
    for (const char* command: {"/dar", "/dar_info", "/invencibilidad"}) {
        EXPECT_NE(info.find(command), std::string::npos);
    }
}

TEST(GiveInvincibilityCheatsTest, InvencibilidadEsIndividualYDesactivarlaRestauraElDesgaste) {
    World world(10, 10);
    add_empty_player(world, 1);
    add_empty_player(world, 2);
    ChatRouter router;
    auto& player = world.get_player(1);
    EXPECT_FALSE(player.is_invincible());
    EXPECT_EQ(give_command(router, world, "/invencibilidad 1"), "invencibilidad activada");
    world.set_current_room(1);
    EXPECT_FALSE(world.damage_player(1, 200));
    world.set_current_room(2);
    EXPECT_EQ(world.get_community_center().goals_for(CommunitySet::COMBAT).back().current, 1);
    player.consume_energy(65535);
    EXPECT_EQ(player.get_health(), 100);
    EXPECT_EQ(player.get_energy(), 10000);
    EXPECT_FALSE(player.diedInCombat());
    EXPECT_FALSE(player.faintedByEnergy());
    EXPECT_TRUE(world.damage_player(2, 200));
    EXPECT_EQ(world.get_player(2).get_health(), 0);
    give_command(router, world, "/invencibilidad 0");
    EXPECT_FALSE(player.is_invincible());
    player.consume_energy(100);
    EXPECT_EQ(player.get_energy(), 9900);
    EXPECT_TRUE(world.damage_player(1, 200));
    EXPECT_TRUE(player.diedInCombat());
}

TEST(GiveInvincibilityCheatsTest, ProtegeSettersCheatsYPenalizacionesSinImpedirCuracionNiFinDeDia) {
    World world(10, 10);
    add_empty_player(world);
    ChatRouter router;
    auto& player = world.get_player(1);
    player.set_health(80);
    player.set_energy(8000);
    give_command(router, world, "/invencibilidad 1");
    EXPECT_EQ(player.get_health(), 80);
    EXPECT_EQ(player.get_energy(), 8000);
    EXPECT_EQ(give_command(router, world, "/vida 0"), "vida: 80");
    EXPECT_EQ(give_command(router, world, "/energia 0"), "energia: 80");
    EXPECT_FALSE(player.diedInCombat());
    EXPECT_FALSE(player.faintedByEnergy());
    player.set_health(90);
    player.set_energy(9000);
    world.mark_all_fainted_by_timeout();
    world.end_day();
    EXPECT_TRUE(world.take_day_ended());
    EXPECT_TRUE(player.faintedByTimeout());
    world.reset_player_for_day(1);
    EXPECT_EQ(player.get_health(), 90);
    EXPECT_EQ(player.get_energy(), 9000);
    EXPECT_TRUE(player.is_invincible());
    give_command(router, world, "/invencibilidad 0");
    player.setFaintedByTimeout(true);
    world.reset_player_for_day(1);
    EXPECT_EQ(player.get_health(), 50);
    EXPECT_EQ(player.get_energy(), 5000);
}

TEST(GiveInvincibilityCheatsTest, ActivacionInvalidaNoCambiaLaInvencibilidad) {
    World world(10, 10);
    add_empty_player(world);
    ChatRouter router;
    for (const char* command: {"/invencibilidad", "/invencibilidad 2", "/invencibilidad -1", "/invencibilidad 1 extra"}) {
        EXPECT_EQ(give_command(router, world, command), "uso: /invencibilidad [1|0]");
        EXPECT_FALSE(world.get_player(1).is_invincible());
    }
    give_command(router, world, "/invencibilidad 1");
    give_command(router, world, "/invencibilidad true");
    EXPECT_TRUE(world.get_player(1).is_invincible());
}

TEST(GiveInvincibilityCheatsTest, DecoracionesConDistintoModeloNoSeMezclanYConservanSuIcono) {
    World world(10, 10);
    add_empty_player(world);
    ChatRouter router;
    give_command(router, world, "/dar decoracion_1 base 2");
    give_command(router, world, "/dar decoracion_2 base");
    const auto& inventory = world.get_player(1).get_inventory();
    EXPECT_EQ(inventory.get_amount(0), 2);
    EXPECT_EQ(inventory.get_amount(1), 1);
    EXPECT_EQ(inventory.peek_item(0)->get_model_id(), 1);
    EXPECT_EQ(inventory.peek_item(1)->get_model_id(), 2);
    EXPECT_EQ(content_id_for(*inventory.peek_item(0)), "mesa_1");
    EXPECT_EQ(content_id_for(*inventory.peek_item(1)), "mesa_2");
    unsigned found = 0;
    for (const auto& message: WorldView{}.sync(world, 1)) {
        if (const auto* slot = std::get_if<ServerInventorySlot>(&message)) {
            if (slot->slot == 0) { EXPECT_EQ(slot->content_id, "mesa_1"); ++found; }
            if (slot->slot == 1) { EXPECT_EQ(slot->content_id, "mesa_2"); ++found; }
        }
    }
    EXPECT_EQ(found, 2u);
}
