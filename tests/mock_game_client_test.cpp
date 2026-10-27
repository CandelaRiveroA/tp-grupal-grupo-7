#include "gtest/gtest.h"

#include <vector>

#include "common/common_command.h"
#include "game/MockGameClient.h"
#include "world/CreatureCatalog.h"

namespace {

protocol::Command walk(int x, int y, protocol::Direction direction) {
	protocol::Command cmd;
	cmd.move = protocol::Move::Walk;
	cmd.x = static_cast<std::uint8_t>(x);
	cmd.y = static_cast<std::uint8_t>(y);
	cmd.direction = direction;
	return cmd;
}

TEST(MockGameClientTest, StartsWithLocalPlayerEntity) {
	game::MockGameClient client(10, 10, 5, 5);
	const game::LocalWorldView& view = client.worldState();
	ASSERT_EQ(view.entities.size(), 1u);
	EXPECT_EQ(view.entities[0].cellX, 5);
	EXPECT_EQ(view.entities[0].cellY, 5);
}

TEST(MockGameClientTest, WalkMovesToProposedCell) {
	game::MockGameClient client(10, 10, 5, 5);
	client.sendCommand(walk(6, 5, protocol::Direction::Right));
	const game::ViewEntity& player = client.worldState().entities[0];
	EXPECT_EQ(player.cellX, 6);
	EXPECT_EQ(player.cellY, 5);
	EXPECT_EQ(player.facing, protocol::Direction::Right);
	EXPECT_TRUE(player.moving);
}

TEST(MockGameClientTest, WalkToAnyCellIsAbsolute) {
	game::MockGameClient client(10, 10, 5, 5);
	client.sendCommand(walk(2, 8, protocol::Direction::Left));
	const game::ViewEntity& player = client.worldState().entities[0];
	EXPECT_EQ(player.cellX, 2);
	EXPECT_EQ(player.cellY, 8);
}

TEST(MockGameClientTest, ClampsTargetOutsideGrid) {
	game::MockGameClient client(10, 10, 5, 5);
	client.sendCommand(walk(99, 99, protocol::Direction::Right));
	const game::ViewEntity& player = client.worldState().entities[0];
	EXPECT_EQ(player.cellX, 9);
	EXPECT_EQ(player.cellY, 9);
}

TEST(MockGameClientTest, WalkToSameCellIsNotMoving) {
	game::MockGameClient client(10, 10, 5, 5);
	client.sendCommand(walk(5, 5, protocol::Direction::Front));
	const game::ViewEntity& player = client.worldState().entities[0];
	EXPECT_FALSE(player.moving);
	EXPECT_EQ(player.facing, protocol::Direction::Front);
}

TEST(MockGameClientTest, PickDoesNotMove) {
	game::MockGameClient client(10, 10, 5, 5);
	protocol::Command pick;
	pick.move = protocol::Move::Pick;
	pick.x = 5;
	pick.y = 5;
	pick.direction = protocol::Direction::Left;
	client.sendCommand(pick);
	const game::ViewEntity& player = client.worldState().entities[0];
	EXPECT_EQ(player.cellX, 5);
	EXPECT_EQ(player.cellY, 5);
	EXPECT_FALSE(player.moving);
	EXPECT_EQ(player.facing, protocol::Direction::Left);
}

TEST(MockGameClientTest, PlayerEntityCarriesItsKind) {
	game::MockGameClient client(10, 10, 5, 5);
	const game::ViewEntity& player = client.worldState().entities[0];
	EXPECT_EQ(player.kind, world::CreatureKind::Player);
}

TEST(MockGameClientTest, ProvidesInitialInventoryTools) {
	game::MockGameClient client(10, 10, 5, 5);
	const std::vector<game::ViewInventorySlot>& inv =
		client.worldState().inventory;
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
	client.sendCommand(walk(6, 5, protocol::Direction::Right));
	EXPECT_EQ(client.worldState().inventory.size(), 4u);
}

TEST(MockGameClientTest, WorldStateReportsGridDimensions) {
	game::MockGameClient client(10, 7, 5, 5);
	const game::LocalWorldView& view = client.worldState();
	EXPECT_EQ(view.grid.columns, 10);
	EXPECT_EQ(view.grid.rows, 7);
}

}
