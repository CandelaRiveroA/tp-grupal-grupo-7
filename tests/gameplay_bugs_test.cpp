#include <chrono>
#include <memory>
#include <thread>
#include <variant>

#include <gtest/gtest.h>

#include "../server/match.h"
#include "../server/domain/entities/dropped_item.h"
#include "../server/domain/inventory/product.h"
#include "../server/domain/inventory/tool.h"
#include "game/SceneTransition.h"
#include "protocol/mock_socket.h"
#include "common/common_protocol_receiver.h"
#include "common/common_protocol_sender.h"

TEST(GameplayBugsTest, EnergiaAlDiezPorCientoReduceElPasoALaMitadYSeRecupera) {
    Player player(1, 1, 1);
    const auto normal = player.get_movement_steps();
    player.set_energy(1001);
    EXPECT_EQ(player.get_movement_steps(), normal);
    player.set_energy(1000);
    EXPECT_EQ(player.get_movement_steps() * 2, normal);
    player.set_energy(1);
    EXPECT_EQ(player.get_movement_steps() * 2, normal);
    player.set_energy(10000);
    EXPECT_EQ(player.get_movement_steps(), normal);
}

TEST(GameplayBugsTest, RecogidaLlenaConservaElHuevoYAvisaSoloAlJugador) {
    World world(6, 6);
    world.add_player(1, std::make_unique<Player>(1, 2, 2));
    auto& player = world.get_player(1);
    // El inventario inicial tiene ocho slots ocupados. Llenar el noveno.
    ASSERT_TRUE(player.pick_up_item(std::make_unique<Product>(ItemID::GOLD_ORB, 10, Quality::BASE)));
    world.set_entity_at(3, 2, std::make_unique<DroppedItem>(
            std::make_unique<Product>(ItemID::EGG, 3, Quality::BASE), 3, 2));
    world.take_item(1);
    EXPECT_TRUE(world.take_pickups().empty());
    EXPECT_EQ(world.take_inventory_full(), (std::set<uint8_t>{1}));
    EXPECT_EQ(world.get_entity_at(3, 2).size(), 1u);
    EXPECT_TRUE(world.take_inventory_full().empty());
    // Un slot ocupado con capacidad de apilar sigue admitiendo la recogida.
    player.select_slot(8);
    player.consume_equipped_item();
    ASSERT_TRUE(player.pick_up_item(std::make_unique<Product>(ItemID::EGG, 3, Quality::BASE)));
    world.take_item(1);
    EXPECT_EQ(world.take_pickups().size(), 1u);
    EXPECT_TRUE(world.get_entity_at(3, 2).empty());
    EXPECT_TRUE(world.take_inventory_full().empty());
}

TEST(GameplayBugsTest, AutoPickupLlenoNoBorraElObjetoDelPiso) {
    World world(8, 8);
    world.add_player(1, std::make_unique<Player>(1, 1, 1));
    ASSERT_TRUE(world.get_player(1).pick_up_item(std::make_unique<Product>(ItemID::GOLD_ORB, 10, Quality::BASE)));
    world.set_entity_at(2, 1, std::make_unique<DroppedItem>(
            std::make_unique<Product>(ItemID::COAL_ORB, 3, Quality::BASE), 2, 1));
    world.move_player(1, Direction::RIGHT);
    EXPECT_EQ(world.take_inventory_full(), (std::set<uint8_t>{1}));
    EXPECT_TRUE(world.take_pickups().empty());
    EXPECT_EQ(world.get_entity_at(2, 1).size(), 1u);
}

TEST(GameplayBugsTest, NuevosMensajesConservanElEncuadreDelProtocolo) {
    MockSocket socket;
    ProtocolSender sender(socket);
    ProtocolReceiver receiver(socket);
    sender.send_server_message(ServerPlayerName{7, "Miguel"});
    sender.send_server_message(ServerInventoryFull{});
    sender.send_server_message(ServerDayEnd{});
    socket.set_input(socket.sent_bytes());
    const auto name = std::get<ServerPlayerName>(receiver.receive_server_message());
    EXPECT_EQ(name.player_id, 7);
    EXPECT_EQ(name.characterName, "Miguel");
    EXPECT_TRUE(std::holds_alternative<ServerInventoryFull>(receiver.receive_server_message()));
    EXPECT_TRUE(std::holds_alternative<ServerDayEnd>(receiver.receive_server_message()));
}

TEST(GameplayBugsTest, FinDeDiaDuranteFadeConservaElAlfaYEsperaEnNegro) {
    game::SceneTransition fade(10);
    fade.changeTo("farm");
    fade.tick(0);
    fade.tick(10);
    fade.consumeSwap();
    fade.tick(14);
    const int before = fade.alpha();
    fade.changeTo("farm", true);
    fade.tick(14);
    EXPECT_EQ(fade.alpha(), before);
    fade.tick(18);
    EXPECT_TRUE(fade.holding());
    fade.tick(100);
    EXPECT_EQ(fade.alpha(), 255);
    fade.changeTo("cabin_1");
    fade.release();
    EXPECT_TRUE(fade.swapDue());
    EXPECT_EQ(fade.scene(), "cabin_1");
}

TEST(GameplayBugsTest, AmanecerRapidoNoDejaElFadeBloqueado) {
    game::SceneTransition fade(10);
    fade.changeTo("farm", true);
    fade.tick(0);
    fade.changeTo("cabin_1");
    fade.release();
    fade.tick(10);
    EXPECT_TRUE(fade.swapDue());
    fade.consumeSwap();
    fade.tick(20);
    EXPECT_FALSE(fade.active());
}

namespace {
worldmodel::World matchWorld() {
    worldmodel::World model;
    for (const auto& id: {"farm", "cabin_1", "cabin_2"}) {
        worldmodel::Map map;
        map.id = id;
        map.kind = map.id == "farm" ? worldmodel::MapKind::Farm : worldmodel::MapKind::Cabin;
        map.width = map.height = 8;
        map.collision.assign(64, false);
        map.baseCollision.assign(64, false);
        model.maps[id] = map;
    }
    worldmodel::MapObject cabin;
    cabin.role = worldmodel::ObjectRole::Cabin;
    model.maps["farm"].objects = {cabin, cabin};
    for (int level = 1; level <= 10; ++level) {
        worldmodel::Map map;
        map.id = "dungeon_" + std::to_string(level);
        map.kind = worldmodel::MapKind::Dungeon;
        map.level = level;
        map.width = map.height = 8;
        map.collision.assign(64, false);
        map.baseCollision.assign(64, false);
        model.maps[map.id] = map;
    }
    return model;
}

class RunningMatch {
public:
    Queue<MatchEvent> events;
    ServerOutputMonitor output;
    Match match;
    RunningMatch(): match(events, output, matchWorld()) { match.start(); }
    ~RunningMatch() { events.close(); match.join(); }
    void join(uint8_t id, const std::shared_ptr<Queue<ServerMessage>>& queue,
              const std::string& name) {
        events.push(PlayerJoined{id, queue, {}, name});
    }
    template<class Predicate>
    bool receive(const std::shared_ptr<Queue<ServerMessage>>& queue, Predicate predicate) {
        const auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(3);
        while (std::chrono::steady_clock::now() < deadline) {
            ServerMessage message = ServerDay{};
            if (queue->try_pop(message)) {
                if (predicate(message)) return true;
            } else {
                std::this_thread::yield();
            }
        }
        return false;
    }
    void sync(const std::shared_ptr<Queue<ServerMessage>>& queue) {
        EXPECT_TRUE(receive(queue, [](const ServerMessage& message) {
            return std::holds_alternative<ServerSyncEnd>(message);
        }));
    }
};
}

TEST(GameplayBugsTest, JoinerRecibeNombresExistentesYLosDemasRecibenAlNuevo) {
    RunningMatch game;
    const auto first = std::make_shared<Queue<ServerMessage>>();
    const auto second = std::make_shared<Queue<ServerMessage>>();
    game.join(1, first, "Ana");
    game.sync(first);
    game.join(2, second, "Miguel");
    const auto nameIs = [](uint8_t id, const std::string& name) {
        return [id, name](const ServerMessage& message) {
            const auto* value = std::get_if<ServerPlayerName>(&message);
            return value && value->player_id == id && value->characterName == name;
        };
    };
    EXPECT_TRUE(game.receive(first, nameIs(2, "Miguel")));
    EXPECT_TRUE(game.receive(second, nameIs(1, "Ana")));
    EXPECT_TRUE(game.receive(second, nameIs(2, "Miguel")));
    game.sync(second);
}

TEST(GameplayBugsTest, Cheat1200TerminaElDiaYDespertarLimpiaElDesmayo) {
    RunningMatch game;
    const auto queue = std::make_shared<Queue<ServerMessage>>();
    game.join(1, queue, "Miguel");
    game.sync(queue);
    game.events.push(PlayerMessage{1, protocol::ChatMessage{"/tiempo 1200"}});
    EXPECT_TRUE(game.receive(queue, [](const ServerMessage& message) {
        return std::holds_alternative<ServerDayEnd>(message);
    }));
    EXPECT_TRUE(game.receive(queue, [](const ServerMessage& message) {
        const auto* status = std::get_if<ServerStatus>(&message);
        return status && status->fainted == 1 && status->health > 0;
    }));
    game.sync(queue);
    game.events.push(PlayerMessage{1, protocol::WakeMessage{}});
    EXPECT_TRUE(game.receive(queue, [](const ServerMessage& message) {
        return std::holds_alternative<ServerDay>(message);
    }));
    EXPECT_TRUE(game.receive(queue, [](const ServerMessage& message) {
        const auto* status = std::get_if<ServerStatus>(&message);
        return status && status->fainted == 0 && status->energy == status->max_energy / 2;
    }));
}

TEST(GameplayBugsTest, PasoTerminalNaturalUsaElMismoCierreDeDia) {
    RunningMatch game;
    const auto queue = std::make_shared<Queue<ServerMessage>>();
    game.join(1, queue, "Miguel");
    game.sync(queue);
    game.events.push(ClockStep{kDayClockMinutes, 0});
    EXPECT_TRUE(game.receive(queue, [](const ServerMessage& message) {
        return std::holds_alternative<ServerDayEnd>(message);
    }));
}

TEST(GameplayBugsTest, PilaParcialConservaElRestoYEmiteUnSoloAviso) {
    World world(6, 6);
    world.add_player(1, std::make_unique<Player>(1, 2, 2));
    auto& player = world.get_player(1);
    for (int i = 0; i < 49; ++i) {
        ASSERT_TRUE(player.pick_up_item(std::make_unique<Product>(ItemID::EGG, 3, Quality::BASE)));
    }
    world.set_entity_at(3, 2, std::make_unique<DroppedItem>(
            std::make_unique<Product>(ItemID::EGG, 3, Quality::BASE), 3, 2, 3));
    world.take_item(1);
    EXPECT_EQ(player.get_inventory().get_amount(8), 50);
    EXPECT_EQ(world.take_pickups().size(), 1u);
    EXPECT_EQ(world.take_inventory_full(), (std::set<uint8_t>{1}));
    EXPECT_EQ(world.get_entity_at(3, 2).size(), 1u);
    player.select_slot(8);
    player.consume_equipped_item();
    player.consume_equipped_item();
    world.take_item(1);
    EXPECT_EQ(player.get_inventory().get_amount(8), 50);
    EXPECT_TRUE(world.get_entity_at(3, 2).empty());
    EXPECT_TRUE(world.take_inventory_full().empty());
}

TEST(GameplayBugsTest, RelojReiniciadoEn1200PublicaElPasoTerminalConSuGeneracion) {
    Queue<MatchEvent> events;
    DayClock clock(events);
    clock.restart(kDayClockMinutes);
    clock.start();
    const auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(2);
    bool terminal = false;
    while (!terminal && std::chrono::steady_clock::now() < deadline) {
        MatchEvent event = CreatureTick{};
        if (!events.try_pop(event)) {
            std::this_thread::yield();
            continue;
        }
        if (const auto* step = std::get_if<ClockStep>(&event)) {
            terminal = step->minutes == kDayClockMinutes && step->generation == clock.generation();
        }
    }
    clock.stop();
    clock.join();
    EXPECT_TRUE(terminal);
}

TEST(GameplayBugsTest, RetrocederElRelojDescartaUnTimeoutDeLaGeneracionAnterior) {
    RunningMatch game;
    const auto queue = std::make_shared<Queue<ServerMessage>>();
    game.join(1, queue, "Miguel");
    game.sync(queue);
    game.events.push(PlayerMessage{1, protocol::ChatMessage{"/tiempo 600"}});
    EXPECT_TRUE(game.receive(queue, [](const ServerMessage& message) {
        const auto* clock = std::get_if<ServerClock>(&message);
        return clock && clock->minutes == 600;
    }));
    game.sync(queue);
    game.events.push(ClockStep{kDayClockMinutes, 0});
    game.events.push(PlayerMessage{1, protocol::ChatMessage{"/tiempo 610"}});
    bool ended = false;
    EXPECT_TRUE(game.receive(queue, [&ended](const ServerMessage& message) {
        ended = ended || std::holds_alternative<ServerDayEnd>(message);
        const auto* clock = std::get_if<ServerClock>(&message);
        return clock && clock->minutes == 610;
    }));
    EXPECT_FALSE(ended);
}

namespace {
void llenarInventario(Player& player) {
    for (const ItemID seed: {ItemID::STRAWBERRY_SEED, ItemID::ONION_SEED,
                             ItemID::POTATO_SEED, ItemID::PUMPKIN_SEED}) {
        for (int i = 0; i < 49; ++i) {
            player.pick_up_item(std::make_unique<Product>(seed, 20, Quality::BASE));
        }
    }
    ASSERT_TRUE(player.pick_up_item(
            std::make_unique<Product>(ItemID::GOLD_ORB, 10, Quality::BASE)));
    EXPECT_EQ(player.get_inventory().get_amount(4), 50u);
    EXPECT_FALSE(player.can_pick_up_item(
            *std::make_unique<Product>(ItemID::STRAWBERRY_SEED, 20, Quality::BASE)));
}
}

TEST(GameplayBugsTest, RegaloConInventarioLlenoAvisaSinQuitarElItem) {
    World world(6, 6);
    world.add_player(1, std::make_unique<Player>(1, 2, 2));
    world.add_player(2, std::make_unique<Player>(2, 3, 2));
    world.select_slot(1, 4);
    llenarInventario(world.get_player(2));
    world.notify_gift_request(1, 2, ItemID::STRAWBERRY_SEED);
    world.process_gift_acceptance(1, 2);
    EXPECT_EQ(world.take_inventory_full(), (std::set<uint8_t>{2}));
    EXPECT_TRUE(world.take_accepted_gifts().empty());
    EXPECT_NE(world.get_player(1).peek_equipped_item(), nullptr);
}

TEST(GameplayBugsTest, RecompensaDeCultivosLlenaSePagaEnOro) {
    World world(6, 6);
    world.add_player(1, std::make_unique<Player>(1, 2, 2));
    Player& player = world.get_player(1);
    llenarInventario(player);
    const uint16_t gold_before = player.get_gold();
    ASSERT_TRUE(world.complete_set("cultivos"));
    EXPECT_EQ(player.get_gold(), static_cast<uint16_t>(gold_before + 20 * 20));
    EXPECT_EQ(world.take_inventory_full(), (std::set<uint8_t>{1}));
}
