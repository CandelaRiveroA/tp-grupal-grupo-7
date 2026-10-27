#include <cstdint>

#include "common/common_command.h"
#include "game/GameClient.h"
#include "game/LocalWorldView.h"
#include "game/MockGameClient.h"
#include "gtest/gtest.h"
#include "world/CreatureCatalog.h"


namespace {

using game::GameClient;
using game::LocalWorldView;
using game::ViewEntity;

game::MockGameClient buildClient(int columns, int rows, int startX, int startY) {
    return game::MockGameClient(columns, rows, startX, startY);
}

protocol::ClientMessage walk(::Direction direction) { return protocol::MoveMessage{direction}; }

bool isValidDirection(world::Direction d) {
    switch (d) {
        case world::Direction::Front:
        case world::Direction::Back:
        case world::Direction::Left:
        case world::Direction::Right:
            return true;
    }
    return false;
}

bool isValidKind(world::CreatureKind k) {
    switch (k) {
        case world::CreatureKind::Player:
            return true;
    }
    return false;
}

TEST(LocalWorldViewContract, EntityZeroIsTheLocalPlayer) {
    game::MockGameClient concrete = buildClient(10, 10, 5, 5);
    GameClient& client = concrete;

    const LocalWorldView& view = client.worldState();
    ASSERT_FALSE(view.entities.empty());
    EXPECT_EQ(view.entities[0].kind, world::CreatureKind::Player);
    EXPECT_EQ(view.entities[0].cellX, 5);
    EXPECT_EQ(view.entities[0].cellY, 5);
}

TEST(LocalWorldViewContract, EntityIndicesAreStableAcrossFrames) {
    game::MockGameClient concrete = buildClient(10, 10, 5, 5);
    GameClient& client = concrete;

    const std::size_t before = client.worldState().entities.size();
    client.sendCommand(walk(::Direction::RIGHT));
    const std::size_t after = client.worldState().entities.size();

    EXPECT_EQ(before, after);
    EXPECT_EQ(client.worldState().entities[0].kind, world::CreatureKind::Player);
}

TEST(LocalWorldViewContract, MovingIsTrueOnAcceptedStep) {
    game::MockGameClient concrete = buildClient(10, 10, 5, 5);
    GameClient& client = concrete;

    client.sendCommand(walk(::Direction::RIGHT));
    EXPECT_TRUE(client.worldState().entities[0].moving);
}

TEST(LocalWorldViewContract, MovingIsFalseWhenStepIsRejected) {
    game::MockGameClient concrete = buildClient(10, 10, 0, 0);
    GameClient& client = concrete;

    client.sendCommand(walk(::Direction::LEFT));
    EXPECT_FALSE(client.worldState().entities[0].moving);
}

TEST(LocalWorldViewContract, MovingIsFalseWhenClampedAgainstTheSameCell) {
    game::MockGameClient concrete = buildClient(10, 10, 0, 0);
    GameClient& client = concrete;

    client.sendCommand(walk(::Direction::UP));
    EXPECT_FALSE(client.worldState().entities[0].moving);
}

TEST(LocalWorldViewContract, MovingIsFalseWhenTargetClampsBackToCurrentCell) {
    game::MockGameClient concrete = buildClient(10, 10, 9, 9);
    GameClient& client = concrete;

    client.sendCommand(walk(::Direction::RIGHT));
    const ViewEntity& player = client.worldState().entities[0];
    EXPECT_EQ(player.cellX, 9);
    EXPECT_EQ(player.cellY, 9);
    EXPECT_FALSE(player.moving);
}

TEST(LocalWorldViewContract, CellStaysInsideTheGrid) {
    game::MockGameClient concrete = buildClient(10, 10, 5, 5);
    GameClient& client = concrete;

    for (int i = 0; i < 20; ++i) {
        client.sendCommand(walk(::Direction::RIGHT));
    }
    const ViewEntity& player = client.worldState().entities[0];
    EXPECT_GE(player.cellX, 0);
    EXPECT_LT(player.cellX, 10);
    EXPECT_GE(player.cellY, 0);
    EXPECT_LT(player.cellY, 10);
}

TEST(LocalWorldViewContract, FacingAndKindAreAlwaysValid) {
    game::MockGameClient concrete = buildClient(10, 10, 5, 5);
    GameClient& client = concrete;

    const ViewEntity& initial = client.worldState().entities[0];
    EXPECT_TRUE(isValidDirection(initial.facing));
    EXPECT_TRUE(isValidKind(initial.kind));

    client.sendCommand(walk(::Direction::RIGHT));
    const ViewEntity& moved = client.worldState().entities[0];
    EXPECT_TRUE(isValidDirection(moved.facing));
    EXPECT_TRUE(isValidKind(moved.kind));
}

TEST(LocalWorldViewContract, ViewIsStableUntilNextCommand) {
    game::MockGameClient concrete = buildClient(10, 10, 5, 5);
    GameClient& client = concrete;

    client.sendCommand(walk(::Direction::RIGHT));
    const LocalWorldView& first = client.worldState();
    const int x = first.entities[0].cellX;
    const int y = first.entities[0].cellY;
    const bool moving = first.entities[0].moving;
    const world::Direction facing = first.entities[0].facing;

    const LocalWorldView& second = client.worldState();
    ASSERT_FALSE(second.entities.empty());
    EXPECT_EQ(second.entities[0].cellX, x);
    EXPECT_EQ(second.entities[0].cellY, y);
    EXPECT_EQ(second.entities[0].moving, moving);
    EXPECT_EQ(second.entities[0].facing, facing);
}

}  // namespace
