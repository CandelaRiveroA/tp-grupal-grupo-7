#include <optional>
#include <stdexcept>
#include <variant>
#include <vector>

#include "gtest/gtest.h"

#include "common/common_command.h"
#include "game/GameClient.h"
#include "game/LocalWorldView.h"
#include "game/PlayerController.h"

namespace {

using game::GameClient;
using game::InputCommand;
using game::LocalWorldView;
using game::PlayerController;
using game::PlayerEvents;
using world::Direction;

const world::MoveCadence kCadence{24, 12};

// GameClient de registro: guarda los comandos enviados para poder
// asertarlos. No simula mundo; el test no lo necesita.
class RecordingGameClient : public GameClient {
public:
    void sendCommand(const protocol::ClientMessage& message) override {
        sent.push_back(message);
    }

    void update(int) override {}

    const LocalWorldView& worldState() const override { return view; }

    std::vector<protocol::ClientMessage> sent;
    LocalWorldView view;
};

InputCommand useTool() {
    return InputCommand{InputCommand::Kind::UseTool, 0};
}

InputCommand collect() {
    return InputCommand{InputCommand::Kind::Collect, 0};
}

InputCommand selectSlot(int slot) {
    return InputCommand{InputCommand::Kind::SelectSlot, slot};
}

TEST(PlayerControllerTest, StepSendsMoveOnWalkCadence) {
    RecordingGameClient client;
    PlayerController player(client, kCadence, 4);

    player.tick(0, Direction::Right, {});
    ASSERT_EQ(client.sent.size(), 1u);
    EXPECT_TRUE(std::holds_alternative<protocol::MoveMessage>(client.sent[0]));

    // Sin pulsación no hay pasos ni comandos.
    client.sent.clear();
    player.tick(1, std::nullopt, {});
    EXPECT_TRUE(client.sent.empty());
}

TEST(PlayerControllerTest, UseToolIsGatedWhileWalking) {
    RecordingGameClient client;
    PlayerController player(client, kCadence, 4);

    // Caminando: el paso va y el golpe no.
    player.tick(0, Direction::Right, {useTool()});
    ASSERT_EQ(client.sent.size(), 1u);
    EXPECT_TRUE(std::holds_alternative<protocol::MoveMessage>(client.sent[0]));

    // Quieto: el golpe va.
    client.sent.clear();
    player.tick(5, std::nullopt, {useTool()});
    ASSERT_EQ(client.sent.size(), 1u);
    EXPECT_TRUE(std::holds_alternative<protocol::PickMessage>(client.sent[0]));
}

TEST(PlayerControllerTest, CollectIsGatedWhileWalking) {
    RecordingGameClient client;
    PlayerController player(client, kCadence, 4);

    player.tick(0, Direction::Right, {collect()});
    ASSERT_EQ(client.sent.size(), 1u);
    EXPECT_TRUE(std::holds_alternative<protocol::MoveMessage>(client.sent[0]));

    client.sent.clear();
    player.tick(5, std::nullopt, {collect()});
    ASSERT_EQ(client.sent.size(), 1u);
    EXPECT_TRUE(std::holds_alternative<protocol::TakeMessage>(client.sent[0]));
}

TEST(PlayerControllerTest, SlotSelectionUpdatesActiveAndRaisesToolSwitch) {
    RecordingGameClient client;
    PlayerController player(client, kCadence, 4);
    EXPECT_EQ(player.activeSlot(), 0);

    const PlayerEvents events = player.tick(0, std::nullopt, {selectSlot(2)});
    EXPECT_EQ(player.activeSlot(), 2);
    EXPECT_TRUE(events.toolSwitched);

    // Re-seleccionar el mismo slot no dispara el evento.
    const PlayerEvents again = player.tick(1, std::nullopt, {selectSlot(2)});
    EXPECT_FALSE(again.toolSwitched);
}

TEST(PlayerControllerTest, CommandsRunInArrivalOrder) {
    RecordingGameClient client;
    PlayerController player(client, kCadence, 4);

    player.tick(0, std::nullopt, {useTool(), collect(), selectSlot(1)});
    ASSERT_EQ(client.sent.size(), 2u);
    EXPECT_TRUE(std::holds_alternative<protocol::PickMessage>(client.sent[0]));
    EXPECT_TRUE(std::holds_alternative<protocol::TakeMessage>(client.sent[1]));
    EXPECT_EQ(player.activeSlot(), 1);
}

TEST(PlayerControllerTest, InteractSendsTake) {
    RecordingGameClient client;
    PlayerController player(client, kCadence, 4);

    // El click sobre algo del mundo manda la intención de tomarlo; la
    // fuente decide qué pasa con ella.
    player.interact();
    ASSERT_EQ(client.sent.size(), 1u);
    EXPECT_TRUE(std::holds_alternative<protocol::TakeMessage>(client.sent[0]));
}

TEST(PlayerControllerTest, UnknownCommandKindThrows) {
    RecordingGameClient client;
    PlayerController player(client, kCadence, 4);

    const InputCommand unknown{static_cast<InputCommand::Kind>(99), 0};
    EXPECT_THROW(player.tick(0, std::nullopt, {unknown}), std::out_of_range);
}

}  // namespace
