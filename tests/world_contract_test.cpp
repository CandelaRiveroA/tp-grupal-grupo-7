#include "gtest/gtest.h"

#include "common/common_command.h"

namespace {


TEST(CommandContractTest, WalkCarriesTargetCellAndDirection) {
	protocol::Command cmd;
	cmd.move = protocol::Move::Walk;
	cmd.x = 6;
	cmd.y = 3;
	cmd.direction = protocol::Direction::Right;
	EXPECT_EQ(cmd.move, protocol::Move::Walk);
	EXPECT_EQ(cmd.x, 6);
	EXPECT_EQ(cmd.y, 3);
	EXPECT_EQ(cmd.direction, protocol::Direction::Right);
}

TEST(CommandContractTest, PickIsAMoveKind) {
	protocol::Command cmd;
	cmd.move = protocol::Move::Pick;
	cmd.x = 0;
	cmd.y = 0;
	cmd.direction = protocol::Direction::Front;
	EXPECT_EQ(cmd.move, protocol::Move::Pick);
}

}
