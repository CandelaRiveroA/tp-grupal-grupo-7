#include <algorithm>
#include <memory>
#include <variant>

#include <gtest/gtest.h>

#include "../server/commands/commands.h"
#include "../server/domain/inventory/product.h"
#include "../server/world_view.h"
#include "common/common_protocol_receiver.h"
#include "common/common_protocol_sender.h"
#include "game/AreaProximity.h"
#include "protocol/mock_socket.h"

namespace {

worldmodel::World community_world() {
    worldmodel::World model;
    worldmodel::Map farm;
    farm.kind = worldmodel::MapKind::Farm;
    farm.width = 30;
    farm.height = 30;
    model.maps["farm"] = farm;
    worldmodel::Map town = farm;
    town.kind = worldmodel::MapKind::Town;
    worldmodel::MapObject center;
    center.role = worldmodel::ObjectRole::CommunityCenter;
    center.cell = {10, 10};
    center.width = 4;
    center.height = 3;
    town.objects.push_back(center);
    model.maps["town"] = town;
    return model;
}

void add_player(World& world, uint8_t id = 1) {
    world.add_player(id, std::make_unique<Player>(id, 11, 13, "town"));
    world.get_player(id).get_inventory() = Inventory(9);
}

uint16_t item_count(const Player& player, ItemID item, uint16_t limit = 20) {
    while (limit > 0 && !player.can_extract_items(item, limit)) --limit;
    return limit;
}

ServerCommunityProgress progress_for(const World& world, uint8_t player, uint8_t set) {
    for (const auto& message: WorldView{}.sync(world, player)) {
        const auto* progress = std::get_if<ServerCommunityProgress>(&message);
        if (progress && progress->set_id == set) return *progress;
    }
    return {};
}

}  // namespace

TEST(CommunityProtocolTest, IntencionSoloEnviaElLoteYRespetaElMensajeSiguiente) {
    MockSocket socket;
    ProtocolSender sender(socket);
    sender.send_client_message(protocol::DonateCommunityMessage{3});
    sender.send_client_message(protocol::WakeMessage{});
    EXPECT_EQ(socket.sent_bytes(), (std::vector<uint8_t>{16, 3, 5}));
    socket.set_input(socket.sent_bytes());
    ProtocolReceiver receiver(socket);
    EXPECT_EQ(std::get<protocol::DonateCommunityMessage>(*receiver.receive_command()).set_id, 3);
    EXPECT_TRUE(std::holds_alternative<protocol::WakeMessage>(*receiver.receive_command()));
}

TEST(CommunityProtocolTest, ProgresoSerializaCantidadesFlagsYLimitesDeMensajes) {
    MockSocket socket;
    ProtocolSender sender(socket);
    sender.send_server_message(ServerCommunityProgress{5, false, {{"money", 1250, 10000, false}}});
    sender.send_server_message(ServerCommunityProgress{3, true,
            {{"milk", 5, 5, true}, {"egg", 10, 10, true}}});
    sender.send_server_message(ServerSyncEnd{});
    socket.set_input(socket.sent_bytes());
    ProtocolReceiver receiver(socket);
    auto first = std::get<ServerCommunityProgress>(receiver.receive_server_message());
    EXPECT_EQ(first.set_id, 5);
    EXPECT_FALSE(first.completed);
    ASSERT_EQ(first.goals.size(), 1u);
    EXPECT_EQ(first.goals[0].goal_id, "money");
    EXPECT_EQ(first.goals[0].current, 1250);
    EXPECT_EQ(first.goals[0].total, 10000);
    EXPECT_FALSE(first.goals[0].completed);
    auto second = std::get<ServerCommunityProgress>(receiver.receive_server_message());
    EXPECT_TRUE(second.completed);
    ASSERT_EQ(second.goals.size(), 2u);
    EXPECT_EQ(second.goals[1].goal_id, "egg");
    EXPECT_TRUE(second.goals[1].completed);
    EXPECT_TRUE(std::holds_alternative<ServerSyncEnd>(receiver.receive_server_message()));
}

TEST(CommunityProtocolTest, AporteParcialConsumeVariosRequisitosSoloDelLoteElegido) {
    World world(community_world());
    add_player(world);
    Player& player = world.get_player(1);
    for (int i = 0; i < 2; ++i) {
        ASSERT_TRUE(player.pick_up_item(std::make_unique<Product>(ItemID::MILK, 10, Quality::BASE)));
        ASSERT_TRUE(player.pick_up_item(std::make_unique<Product>(ItemID::EGG, 10, Quality::BASE)));
    }
    ASSERT_TRUE(player.pick_up_item(std::make_unique<Product>(ItemID::COAL_ORB, 10, Quality::BASE)));
    commands::apply(world, 1, protocol::DonateCommunityMessage{3});
    const auto progress = progress_for(world, 1, 3);
    ASSERT_EQ(progress.goals.size(), 2u);
    EXPECT_EQ(progress.goals[0].current, 2);
    EXPECT_EQ(progress.goals[1].current, 2);
    EXPECT_FALSE(progress.completed);
    EXPECT_EQ(item_count(player, ItemID::MILK), 0);
    EXPECT_EQ(item_count(player, ItemID::EGG), 0);
    EXPECT_EQ(item_count(player, ItemID::COAL_ORB), 1);
    EXPECT_FALSE(world.donate_to_community_center(1, CommunitySet::ANIMALS));
}

TEST(CommunityProtocolTest, NoConsumeExcedentesYComparteProgresoConQuienEntraDespues) {
    World world(community_world());
    add_player(world);
    for (int i = 0; i < 7; ++i) {
        ASSERT_TRUE(world.get_player(1).pick_up_item(
                std::make_unique<Product>(ItemID::MILK, 10, Quality::BASE)));
    }
    EXPECT_TRUE(world.donate_to_community_center(1, CommunitySet::ANIMALS));
    EXPECT_EQ(item_count(world.get_player(1), ItemID::MILK), 2);
    add_player(world, 2);
    const auto progress = progress_for(world, 2, 3);
    ASSERT_EQ(progress.goals.size(), 2u);
    EXPECT_EQ(progress.goals[0].current, 5);
    EXPECT_TRUE(progress.goals[0].completed);
    EXPECT_FALSE(world.donate_to_community_center(1, CommunitySet::ANIMALS));
    EXPECT_EQ(item_count(world.get_player(1), ItemID::MILK), 2);
}

TEST(CommunityProtocolTest, CultivosAceptaIncreibleYPerfectaSinConsumirCalidadBaja) {
    World world(community_world());
    add_player(world);
    auto& player = world.get_player(1);
    for (Quality quality: {Quality::BASE, Quality::INCREDIBLE, Quality::PERFECT}) {
        ASSERT_TRUE(player.pick_up_item(std::make_unique<Product>(ItemID::STRAWBERRY, 10, quality)));
    }
    EXPECT_TRUE(world.donate_to_community_center(1, CommunitySet::CROPS));
    EXPECT_EQ(item_count(player, ItemID::STRAWBERRY), 1);
    EXPECT_EQ(progress_for(world, 1, 1).goals[0].current, 2);
    EXPECT_FALSE(world.donate_to_community_center(1, CommunitySet::CROPS));
}

TEST(CommunityProtocolTest, DineroAportaMilPermiteSaldoParcialYLimitaAlPendiente) {
    World world(community_world());
    add_player(world);
    auto& player = world.get_player(1);
    player.set_gold(2500);
    EXPECT_TRUE(world.donate_to_community_center(1, CommunitySet::MONEY));
    EXPECT_EQ(player.get_gold(), 1500);
    EXPECT_EQ(world.get_community_center().get_gold_donated(), 1000u);
    EXPECT_TRUE(world.donate_to_community_center(1, CommunitySet::MONEY));
    EXPECT_TRUE(world.donate_to_community_center(1, CommunitySet::MONEY));
    EXPECT_EQ(player.get_gold(), 0);
    EXPECT_EQ(world.get_community_center().get_gold_donated(), 2500u);
    EXPECT_FALSE(world.donate_to_community_center(1, CommunitySet::MONEY));
    world.record_gold_donated(7250);
    player.set_gold(2000);
    EXPECT_TRUE(world.donate_to_community_center(1, CommunitySet::MONEY));
    EXPECT_EQ(player.get_gold(), 1750);
    EXPECT_TRUE(world.is_set_completed("dinero"));
    EXPECT_TRUE(world.is_market_discount_active());
    EXPECT_TRUE(world.is_double_sell_active());
    EXPECT_EQ(world.take_completed_sets(), (std::vector<uint8_t>{5}));
    EXPECT_FALSE(world.donate_to_community_center(1, CommunitySet::MONEY));
    EXPECT_EQ(player.get_gold(), 1750);
}

TEST(CommunityProtocolTest, NoDescuentaOroCuandoElLoteYaSuperoElObjetivo) {
    World world(community_world());
    add_player(world);
    auto& player = world.get_player(1);
    player.set_gold(100);
    world.get_community_center().offer_gold(11000);

    EXPECT_FALSE(world.donate_to_community_center(1, "dinero", ItemID::GOLD_ORB, 100));
    EXPECT_EQ(player.get_gold(), 100u);
    EXPECT_EQ(world.get_community_center().get_gold_donated(), 11000u);
}

TEST(CommunityProtocolTest, ServidorUsaElMismoAnilloUnoQueElPopup) {
    World world(community_world());
    add_player(world);
    auto& player = world.get_player(1);
    player.set_gold(60000);
    for (uint16_t y = 8; y <= 14; ++y) {
        for (uint16_t x = 8; x <= 15; ++x) {
            world.transfer_player(1, "town", x, y);
            const bool near = game::AreaProximity(1).around({10, 10}, {4, 3}, x, y);
            EXPECT_EQ(world.donate_to_community_center(1, CommunitySet::CROPS), false);
            // Usar un saldo parcial para no completar el lote durante el recorrido.
            player.set_gold(1);
            EXPECT_EQ(world.donate_to_community_center(1, CommunitySet::MONEY), near);
        }
    }
    player.set_gold(1000);
    world.transfer_player(1, "farm", 11, 13);
    EXPECT_FALSE(world.donate_to_community_center(1, CommunitySet::MONEY));
    EXPECT_EQ(player.get_gold(), 1000);
}

TEST(CommunityProtocolTest, LotesInvalidosDePeleaYCompletadosNoConsumen) {
    World world(community_world());
    add_player(world);
    auto& player = world.get_player(1);
    player.set_gold(2000);
    EXPECT_FALSE(world.donate_to_community_center(1, static_cast<CommunitySet>(0)));
    EXPECT_FALSE(world.donate_to_community_center(1, static_cast<CommunitySet>(255)));
    EXPECT_FALSE(world.donate_to_community_center(1, CommunitySet::COMBAT));
    world.complete_set("dinero");
    EXPECT_FALSE(world.donate_to_community_center(1, CommunitySet::MONEY));
    EXPECT_EQ(player.get_gold(), 2000);
    const auto completed = progress_for(world, 1, 5);
    EXPECT_TRUE(completed.completed);
    EXPECT_EQ(completed.goals[0].current, completed.goals[0].total);
}

TEST(CommunityProtocolTest, ProgresoDeCombateConservaGruposParejasYSalas) {
    World world(community_world());
    add_player(world);
    world.record_enemy_killed("skeleton", 0, 0, 1);
    world.record_enemy_killed("skeleton", 0, 0, 2);
    world.record_enemy_killed("orc", 1, 0, 1);
    world.record_enemy_killed("orc", 2, 0, 1);
    world.record_enemy_killed("vampire", 0, 1, 1);
    const auto progress = progress_for(world, 1, 4);
    ASSERT_EQ(progress.goals.size(), 4u);
    EXPECT_EQ(progress.goals[0].current, 2);
    EXPECT_EQ(progress.goals[1].current, 1);
    EXPECT_EQ(progress.goals[2].current, 1);
    EXPECT_FALSE(progress.completed);
    world.record_enemy_killed("skeleton", 0, 0, 3);
    for (int i = 0; i < 3; ++i) world.record_enemy_killed("orc", 1, 0, 3);
    world.record_enemy_killed("vampiress", 0, 1, 3);
    world.get_community_center().record_skull_survival(true);
    world.record_enemy_killed("skeleton", 0, 0, 0);
    EXPECT_TRUE(world.is_set_completed("pelea"));
}

TEST(CommunityProtocolTest, DosJugadoresCompletanElMismoLoteSinConsumirExcedentesNiRepetirPremio) {
    World world(community_world());
    add_player(world, 1);
    add_player(world, 2);
    for (int i = 0; i < 6; ++i) {
        ASSERT_TRUE(world.get_player(1).pick_up_item(
                std::make_unique<Product>(ItemID::MILK, 10, Quality::BASE)));
    }
    for (int i = 0; i < 12; ++i) {
        ASSERT_TRUE(world.get_player(2).pick_up_item(
                std::make_unique<Product>(ItemID::EGG, 10, Quality::BASE)));
    }
    EXPECT_TRUE(world.donate_to_community_center(1, CommunitySet::ANIMALS));
    EXPECT_FALSE(world.is_set_completed("animales"));
    EXPECT_TRUE(world.donate_to_community_center(2, CommunitySet::ANIMALS));
    EXPECT_TRUE(world.is_set_completed("animales"));
    EXPECT_EQ(item_count(world.get_player(1), ItemID::MILK), 1);
    EXPECT_EQ(item_count(world.get_player(2), ItemID::EGG), 2);
    EXPECT_TRUE(progress_for(world, 1, 3).completed);
    EXPECT_TRUE(progress_for(world, 2, 3).completed);
    EXPECT_EQ(world.take_completed_sets(), (std::vector<uint8_t>{3}));
    const uint16_t gold = world.get_player(2).get_gold();
    EXPECT_FALSE(world.donate_to_community_center(2, CommunitySet::ANIMALS));
    EXPECT_EQ(world.get_player(2).get_gold(), gold);
    EXPECT_TRUE(world.take_completed_sets().empty());
}
