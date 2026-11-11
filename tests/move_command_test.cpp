#include "gtest/gtest.h"
#include "game/MoveCommand.h"
#include "world/CreatureCatalog.h"

namespace {

using game::MoveCommand;

TEST(MoveCommandTest, ProposeBuildsMoveWithWireDirection) {
	const MoveCommand move;
	const protocol::ClientMessage message =
		move.propose(world::Direction::Right);
	const auto* walk = std::get_if<protocol::MoveMessage>(&message);
	ASSERT_NE(walk, nullptr);
	EXPECT_EQ(walk->direction, ::Direction::RIGHT);
}

TEST(MoveCommandTest, ProposeMapsBackToUp) {
	const MoveCommand move;
	const protocol::ClientMessage message =
		move.propose(world::Direction::Back);
	const auto* walk = std::get_if<protocol::MoveMessage>(&message);
	ASSERT_NE(walk, nullptr);
	EXPECT_EQ(walk->direction, ::Direction::UP);
}

TEST(MoveCommandTest, ProposeMapsFrontToDown) {
	const MoveCommand move;
	const protocol::ClientMessage message =
		move.propose(world::Direction::Front);
	const auto* walk = std::get_if<protocol::MoveMessage>(&message);
	ASSERT_NE(walk, nullptr);
	EXPECT_EQ(walk->direction, ::Direction::DOWN);
}

TEST(MoveCommandTest, UseBuildsPickWithFacingDirection) {
	const MoveCommand move;
	const protocol::ClientMessage message =
		move.use(world::Direction::Left, 0);
	const auto* pick = std::get_if<protocol::PickMessage>(&message);
	ASSERT_NE(pick, nullptr);
	EXPECT_EQ(pick->direction, ::Direction::LEFT);
}

TEST(MoveCommandTest, TakeBuildsTakeWithFacingDirection) {
	const MoveCommand move;
	const protocol::ClientMessage message =
		move.take(world::Direction::Right);
	const auto* take = std::get_if<protocol::TakeMessage>(&message);
	ASSERT_NE(take, nullptr);
	EXPECT_EQ(take->direction, ::Direction::RIGHT);
}

}
