#include "gtest/gtest.h"
#include "game/MoveCommand.h"
#include "world/CreatureCatalog.h"

namespace {

using game::LocalWorldView;
using game::MoveCommand;
using game::ViewEntity;

LocalWorldView viewAt(int cellX, int cellY) {
	LocalWorldView view;
	view.entities.push_back(
		ViewEntity{world::CreatureKind::Player, cellX, cellY,
			   protocol::Direction::Front, false,
			   world::ActionKind::Idle, std::nullopt});
	return view;
}


TEST(MoveCommandTest, ProposesTheAdjacentCellToTheRight) {
	const MoveCommand move;
	const LocalWorldView view = viewAt(3, 4);
	const protocol::Command command =
		move.propose(view, protocol::Direction::Right);
	EXPECT_EQ(command.x, 4);
	EXPECT_EQ(command.y, 4);
}

TEST(MoveCommandTest, ProposesTheAdjacentCellUpTheGrid) {
	const MoveCommand move;
	const LocalWorldView view = viewAt(3, 4);
	const protocol::Command command =
		move.propose(view, protocol::Direction::Back);
	EXPECT_EQ(command.x, 3);
	EXPECT_EQ(command.y, 3);
}

TEST(MoveCommandTest, CarriesDirectionAndWalkMove) {
	const MoveCommand move;
	const LocalWorldView view = viewAt(1, 1);
	const protocol::Command command =
		move.propose(view, protocol::Direction::Front);
	EXPECT_EQ(command.direction, protocol::Direction::Front);
	EXPECT_EQ(command.move, protocol::Move::Walk);
}

TEST(MoveCommandTest, ClampsAtTheOriginInsteadOfWrapping) {
	const MoveCommand move;
	const LocalWorldView view = viewAt(0, 0);
	const protocol::Command command =
		move.propose(view, protocol::Direction::Left);
	EXPECT_EQ(command.x, 0);
	EXPECT_EQ(command.y, 0);
}

TEST(MoveCommandTest, EmptyViewStartsFromTheOrigin) {
	const MoveCommand move;
	const LocalWorldView empty;
	const protocol::Command command =
		move.propose(empty, protocol::Direction::Right);
	EXPECT_EQ(command.x, 1);
	EXPECT_EQ(command.y, 0);
}


TEST(MoveCommandTest, HoldProposesTheCurrentCell) {
	const MoveCommand move;
	const LocalWorldView view = viewAt(3, 4);
	const protocol::Command command =
		move.hold(view, protocol::Direction::Right);
	EXPECT_EQ(command.x, 3);
	EXPECT_EQ(command.y, 4);
	EXPECT_EQ(command.move, protocol::Move::Walk);
	EXPECT_EQ(command.direction, protocol::Direction::Right);
}

TEST(MoveCommandTest, HoldAtTheBorderStaysAtTheOrigin) {
	const MoveCommand move;
	const LocalWorldView view = viewAt(0, 0);
	const protocol::Command command =
		move.hold(view, protocol::Direction::Left);
	EXPECT_EQ(command.x, 0);
	EXPECT_EQ(command.y, 0);
}

}
