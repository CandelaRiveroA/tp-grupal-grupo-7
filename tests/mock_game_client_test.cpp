#include <vector>

#include "common/common_command.h"
#include "game/MockGameClient.h"
#include "gtest/gtest.h"
#include "world/CreatureCatalog.h"

namespace {

protocol::ClientMessage walk(::Direction direction) { return protocol::MoveMessage{direction}; }

TEST(MockGameClientTest, StartsWithLocalPlayerEntity) {
    game::MockGameClient client(10, 10, 5, 5);
    const game::LocalWorldView& view = client.worldState();
    ASSERT_EQ(view.entities.size(), 1u);
    EXPECT_EQ(view.entities[0].cellX, 5);
    EXPECT_EQ(view.entities[0].cellY, 5);
}

TEST(MockGameClientTest, WalkMovesOneCellInTheGivenDirection) {
    game::MockGameClient client(10, 10, 5, 5);
    client.sendCommand(walk(::Direction::RIGHT));
    const game::ViewEntity& player = client.worldState().entities[0];
    EXPECT_EQ(player.cellX, 6);
    EXPECT_EQ(player.cellY, 5);
    EXPECT_EQ(player.facing, world::Direction::Right);
    EXPECT_TRUE(player.moving);
}

TEST(MockGameClientTest, UpDecreasesYAndFacesBack) {
    game::MockGameClient client(10, 10, 5, 5);
    client.sendCommand(walk(::Direction::UP));
    const game::ViewEntity& player = client.worldState().entities[0];
    EXPECT_EQ(player.cellX, 5);
    EXPECT_EQ(player.cellY, 4);
    EXPECT_EQ(player.facing, world::Direction::Back);
}

TEST(MockGameClientTest, DownIncreasesYAndFacesFront) {
    game::MockGameClient client(10, 10, 5, 5);
    client.sendCommand(walk(::Direction::DOWN));
    const game::ViewEntity& player = client.worldState().entities[0];
    EXPECT_EQ(player.cellX, 5);
    EXPECT_EQ(player.cellY, 6);
    EXPECT_EQ(player.facing, world::Direction::Front);
}

TEST(MockGameClientTest, ClampsMovementAtTheGridBorder) {
    game::MockGameClient client(10, 10, 0, 0);
    client.sendCommand(walk(::Direction::LEFT));
    client.sendCommand(walk(::Direction::UP));
    const game::ViewEntity& player = client.worldState().entities[0];
    EXPECT_EQ(player.cellX, 0);
    EXPECT_EQ(player.cellY, 0);
    EXPECT_FALSE(player.moving);
    EXPECT_EQ(player.facing, world::Direction::Back);
}

TEST(MockGameClientTest, PlayerEntityCarriesItsKind) {
    game::MockGameClient client(10, 10, 5, 5);
    const game::ViewEntity& player = client.worldState().entities[0];
    EXPECT_EQ(player.kind, world::CreatureKind::Player);
}

TEST(MockGameClientTest, ProvidesInitialInventoryTools) {
    game::MockGameClient client(10, 10, 5, 5);
    const std::vector<game::ViewInventorySlot>& inv = client.worldState().inventory;
    ASSERT_EQ(inv.size(), 4u);
    EXPECT_EQ(inv[0].slotIndex, 0);
    EXPECT_EQ(inv[0].item, world::ItemKind::Shovel);
    EXPECT_EQ(inv[1].slotIndex, 1);
    EXPECT_EQ(inv[1].item, world::ItemKind::Pickaxe);
    EXPECT_EQ(inv[2].slotIndex, 2);
    EXPECT_EQ(inv[2].item, world::ItemKind::Sword);
    EXPECT_EQ(inv[3].slotIndex, 3);
    EXPECT_EQ(inv[3].item, world::ItemKind::WateringCan);
}

TEST(MockGameClientTest, InventoryStaysWhenPlayerMoves) {
    game::MockGameClient client(10, 10, 5, 5);
    client.sendCommand(walk(::Direction::RIGHT));
    EXPECT_EQ(client.worldState().inventory.size(), 4u);
}

}  // namespace
