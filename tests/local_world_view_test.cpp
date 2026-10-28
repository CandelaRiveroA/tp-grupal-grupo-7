#include <cstdint>

#include "gtest/gtest.h"

#include "common/common_command.h"
#include "game/GameClient.h"
#include "game/LocalWorldView.h"
#include "game/MockGameClient.h"
#include "world/CreatureCatalog.h"


namespace {

using game::GameClient;
using game::LocalWorldView;
using game::ViewEntity;

game::MockGameClient buildClient(int columns, int rows, int startX,
				 int startY) {
	return game::MockGameClient(columns, rows, startX, startY);
}

protocol::Command walk(int x, int y, protocol::Direction direction) {
	protocol::Command cmd;
	cmd.move = protocol::Move::Walk;
	cmd.x = static_cast<std::uint8_t>(x);
	cmd.y = static_cast<std::uint8_t>(y);
	cmd.direction = direction;
	return cmd;
}

bool isValidDirection(protocol::Direction d) {
	switch (d) {
	case protocol::Direction::Front:
	case protocol::Direction::Back:
	case protocol::Direction::Left:
	case protocol::Direction::Right:
		return true;
	}
	return false;
}

bool isValidKind(world::CreatureKind k) {
	switch (k) {
	case world::CreatureKind::Player:
	case world::CreatureKind::Chicken:
	case world::CreatureKind::Cow:
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
	client.sendCommand(walk(6, 5, protocol::Direction::Right));
	const std::size_t after = client.worldState().entities.size();

	EXPECT_EQ(before, after);
	EXPECT_EQ(client.worldState().entities[0].kind,
		  world::CreatureKind::Player);
}

TEST(LocalWorldViewContract, MovingIsTrueOnAcceptedStep) {
	game::MockGameClient concrete = buildClient(10, 10, 5, 5);
	GameClient& client = concrete;

	client.sendCommand(walk(6, 5, protocol::Direction::Right));
	EXPECT_TRUE(client.worldState().entities[0].moving);
}

TEST(LocalWorldViewContract, MovingIsFalseWhenStepIsRejected) {
	game::MockGameClient concrete = buildClient(10, 10, 5, 5);
	GameClient& client = concrete;

	client.sendCommand(walk(5, 5, protocol::Direction::Front));
	EXPECT_FALSE(client.worldState().entities[0].moving);
}

TEST(LocalWorldViewContract, MovingIsFalseWhenClampedAgainstTheSameCell) {
	game::MockGameClient concrete = buildClient(10, 10, 0, 0);
	GameClient& client = concrete;

	client.sendCommand(walk(0, 0, protocol::Direction::Left));
	EXPECT_FALSE(client.worldState().entities[0].moving);
}

TEST(LocalWorldViewContract, MovingIsFalseWhenTargetClampsBackToCurrentCell) {
	game::MockGameClient concrete = buildClient(10, 10, 9, 9);
	GameClient& client = concrete;

	client.sendCommand(walk(99, 99, protocol::Direction::Right));
	const ViewEntity& player = client.worldState().entities[0];
	EXPECT_EQ(player.cellX, 9);
	EXPECT_EQ(player.cellY, 9);
	EXPECT_FALSE(player.moving);
}

TEST(LocalWorldViewContract, CellStaysInsideTheGrid) {
	game::MockGameClient concrete = buildClient(10, 10, 5, 5);
	GameClient& client = concrete;

	client.sendCommand(walk(999, 999, protocol::Direction::Right));
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

	client.sendCommand(walk(6, 5, protocol::Direction::Right));
	const ViewEntity& moved = client.worldState().entities[0];
	EXPECT_TRUE(isValidDirection(moved.facing));
	EXPECT_TRUE(isValidKind(moved.kind));
}

TEST(LocalWorldViewContract, ViewIsStableUntilNextCommand) {
	game::MockGameClient concrete = buildClient(10, 10, 5, 5);
	GameClient& client = concrete;

	client.sendCommand(walk(6, 5, protocol::Direction::Right));
	const LocalWorldView& first = client.worldState();
	const int x = first.entities[0].cellX;
	const int y = first.entities[0].cellY;
	const bool moving = first.entities[0].moving;
	const protocol::Direction facing = first.entities[0].facing;

	const LocalWorldView& second = client.worldState();
	ASSERT_FALSE(second.entities.empty());
	EXPECT_EQ(second.entities[0].cellX, x);
	EXPECT_EQ(second.entities[0].cellY, y);
	EXPECT_EQ(second.entities[0].moving, moving);
	EXPECT_EQ(second.entities[0].facing, facing);
}

TEST(LocalWorldViewContract, LocalPlayerCarriesStableIdZero) {
	game::MockGameClient concrete = buildClient(10, 10, 5, 5);
	GameClient& client = concrete;

	EXPECT_EQ(client.worldState().entities[0].id, 0);
	client.sendCommand(walk(6, 5, protocol::Direction::Right));
	EXPECT_EQ(client.worldState().entities[0].id, 0);
}

}
