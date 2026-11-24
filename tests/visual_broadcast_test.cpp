#include <memory>
#include <optional>
#include <string>
#include <vector>

#include <gtest/gtest.h>

#include "../server/commands/commands.h"
#include "../server/domain/inventory/product.h"
#include "../server/world_view.h"
#include "game/ActionResolver.h"
#include "game/GameClient.h"
#include "game/GiftThanks.h"
#include "game/PlayerController.h"

namespace {

class GiftScenario {
public:
    World world;
    GiftScenario(): world(6, 6) {
        world.add_player(1, std::make_unique<Player>(1, 1, 1));
        world.add_player(2, std::make_unique<Player>(2, 2, 1));
        world.select_slot(1, 4);
    }
    void offer() {
        const Item* item = world.get_player(1).peek_equipped_item();
        world.notify_gift_request(1, 2, item->get_id());
    }
    std::optional<std::string> held(uint8_t viewer, uint8_t player) {
        for (const ServerMessage& message: WorldView().sync(world, viewer)) {
            if (const auto* state = std::get_if<ServerPlayerHeld>(&message)) {
                if (state->player_id == player) return state->content_id;
            }
        }
        return std::nullopt;
    }
};

class RecordingClient: public game::GameClient {
public:
    game::LocalWorldView view;
    std::vector<protocol::ClientMessage> commands;
    void sendCommand(const protocol::ClientMessage& message) override {
        commands.push_back(message);
    }
    const game::LocalWorldView& worldState() const override { return view; }
    void update(int) override {}
};

TEST(VisualBroadcastTest, SeleccionarSlotEnviaLaIntencionSinUsarElObjeto) {
    RecordingClient client;
    game::PlayerController controller(client, world::MoveCadence{1, 1}, 9);
    controller.tick(0, std::nullopt, {{game::InputCommand::Kind::SelectSlot, 4}});
    ASSERT_EQ(client.commands.size(), 1u);
    ASSERT_TRUE(std::holds_alternative<protocol::SelectSlotMessage>(client.commands[0]));
    EXPECT_EQ(std::get<protocol::SelectSlotMessage>(client.commands[0]).slot, 4);
    controller.tick(1, std::nullopt, {{game::InputCommand::Kind::SelectSlot, 4}});
    controller.tick(2, std::nullopt, {{game::InputCommand::Kind::SelectSlot, 20}});
    EXPECT_EQ(client.commands.size(), 1u);
}

TEST(VisualBroadcastTest, SeleccionPublicaEsLaMismaParaTodosLosObservadores) {
    GiftScenario game;
    commands::apply(game.world, 1, protocol::SelectSlotMessage{5});
    EXPECT_EQ(game.held(1, 1), std::optional<std::string>{"onion_seeds"});
    EXPECT_EQ(game.held(2, 1), game.held(1, 1));
    commands::apply(game.world, 1, protocol::SelectSlotMessage{0});
    EXPECT_EQ(game.held(2, 1), std::optional<std::string>{""});
}

TEST(VisualBroadcastTest, ManoRemotaSeVaciaAlRegalarLaUltimaUnidad) {
    GiftScenario game;
    EXPECT_EQ(game.held(2, 1), std::optional<std::string>{"strawberry_seeds"});
    game.offer();
    commands::apply(game.world, 2, protocol::GiftResponseMessage{1, true});
    EXPECT_EQ(game.held(2, 1), std::optional<std::string>{""});
    const auto events = game.world.take_accepted_gifts();
    ASSERT_EQ(events.size(), 1u);
    EXPECT_EQ(events[0], 2);
    EXPECT_TRUE(game.world.take_accepted_gifts().empty());
}

TEST(VisualBroadcastTest, SeleccionInvalidaNoCambiaLoSostenido) {
    GiftScenario game;
    commands::apply(game.world, 1, protocol::SelectSlotMessage{255});
    EXPECT_EQ(game.held(2, 1), std::optional<std::string>{"strawberry_seeds"});
}

TEST(VisualBroadcastTest, RegaloRechazadoNoProduceCorazonNiTransferenciaPosterior) {
    GiftScenario game;
    game.offer();
    commands::apply(game.world, 2, protocol::GiftResponseMessage{1, false});
    EXPECT_TRUE(game.world.take_accepted_gifts().empty());
    commands::apply(game.world, 2, protocol::GiftResponseMessage{1, true});
    EXPECT_TRUE(game.world.take_accepted_gifts().empty());
    EXPECT_EQ(game.held(2, 1), std::optional<std::string>{"strawberry_seeds"});
}

TEST(VisualBroadcastTest, InventarioLlenoNoProduceUnCorazonFalso) {
    GiftScenario game;
    game.world.get_player(1).pick_up_item(
            std::make_unique<Product>(ItemID::STRAWBERRY, 80, Quality::BASE));
    game.world.get_player(2).pick_up_item(
            std::make_unique<Product>(ItemID::EGG, 3, Quality::BASE));
    game.world.select_slot(1, 8);
    game.offer();
    commands::apply(game.world, 2, protocol::GiftResponseMessage{1, true});
    EXPECT_TRUE(game.world.take_accepted_gifts().empty());
    EXPECT_FALSE(game.world.get_player(1).get_inventory().is_slot_empty(8));
}

TEST(VisualBroadcastTest, NoSeRegalaOtroObjetoSiCambioLaSeleccionDuranteLaOferta) {
    GiftScenario game;
    game.offer();
    game.world.select_slot(1, 5);
    commands::apply(game.world, 2, protocol::GiftResponseMessage{1, true});
    EXPECT_TRUE(game.world.take_accepted_gifts().empty());
    EXPECT_FALSE(game.world.get_player(1).get_inventory().is_slot_empty(4));
    EXPECT_FALSE(game.world.get_player(1).get_inventory().is_slot_empty(5));
}

TEST(VisualBroadcastTest, RespuestaSinOfertaYRespuestaDuplicadaNoRepitenRegalos) {
    GiftScenario game;
    commands::apply(game.world, 2, protocol::GiftResponseMessage{1, true});
    EXPECT_TRUE(game.world.take_accepted_gifts().empty());
    game.offer();
    commands::apply(game.world, 2, protocol::GiftResponseMessage{1, true});
    ASSERT_EQ(game.world.take_accepted_gifts().size(), 1u);
    commands::apply(game.world, 2, protocol::GiftResponseMessage{1, true});
    EXPECT_TRUE(game.world.take_accepted_gifts().empty());
}

TEST(VisualBroadcastTest, CorazonSeAplicaAlReceptorLocalORemotoPeroNoAlAnimalConMismoId) {
    game::LocalWorldView base;
    game::ViewEntity first{};
    first.kind = world::CreatureKind::Player;
    first.id = 1;
    game::ViewEntity receiver = first;
    receiver.id = 2;
    game::ViewEntity animal = receiver;
    animal.kind = world::CreatureKind::Cow;
    base.entities = {first, receiver, animal};
    game::GiftThanks thanks;
    thanks.update({2}, 2);
    auto drawn = base;
    thanks.apply(drawn);
    EXPECT_FALSE(drawn.entities[0].indication.has_value());
    EXPECT_EQ(drawn.entities[1].indication, world::Indication::GiftThanks);
    EXPECT_FALSE(drawn.entities[2].indication.has_value());
    std::swap(base.entities[0], base.entities[1]);
    drawn = base;
    thanks.apply(drawn);
    EXPECT_EQ(drawn.entities[0].indication, world::Indication::GiftThanks);
    thanks.update({}, 2);
    thanks.update({}, 2);
    drawn = base;
    thanks.apply(drawn);
    EXPECT_FALSE(drawn.entities[0].indication.has_value());
}

TEST(VisualBroadcastTest, ObjetoRemotoYObjetoLocalNoSeMezclan) {
    world::ContentTraits traits;
    traits.set("strawberry_seeds", world::ContentTrait{true});
    game::CarrySelection carry(traits);
    game::LocalWorldView inventory;
    inventory.placeInventoryContent(4, "strawberry_seeds", 1);
    carry.observe(inventory, 4);
    const world::CharacterCatalog characters;
    const game::ViewMotion motion(characters);
    const game::ActionResolver actions(motion, carry);
    game::ViewEntity remote{};
    remote.kind = world::CreatureKind::Player;
    remote.action = world::ActionKind::Idle;
    remote.carriedContent = "egg";
    EXPECT_EQ(actions.actionFor(remote, 1, 0), world::ActionKind::Carry);
    EXPECT_EQ(actions.heldContentId(remote, 1), std::optional<std::string>{"egg"});
    EXPECT_EQ(actions.heldContentId(remote, 0),
              std::optional<std::string>{"strawberry_seeds"});
    remote.moving = true;
    EXPECT_EQ(actions.actionFor(remote, 1, 0), world::ActionKind::CarryWalk);
    remote.carriedContent.reset();
    remote.moving = false;
    EXPECT_EQ(actions.actionFor(remote, 1, 0), world::ActionKind::Idle);
}

}  // namespace
