#include "game/MoveCommand.h"
#include "gtest/gtest.h"

namespace {

using game::MoveCommand;

TEST(MoveCommandTest, ProposesAMoveWithTheGivenDirection) {
    const MoveCommand move;
    const protocol::ClientMessage message = move.propose(world::Direction::Front);
    const auto* walk = std::get_if<protocol::MoveMessage>(&message);
    ASSERT_NE(walk, nullptr);
    EXPECT_EQ(walk->direction, ::Direction::DOWN);
}

TEST(MoveCommandTest, MapsBackToUpAndLeftRightToTheirSameNames) {
    const MoveCommand move;
    const auto directionOf = [&move](world::Direction direction) {
        return std::get<protocol::MoveMessage>(move.propose(direction)).direction;
    };
    EXPECT_EQ(directionOf(world::Direction::Back), ::Direction::UP);
    EXPECT_EQ(directionOf(world::Direction::Left), ::Direction::LEFT);
    EXPECT_EQ(directionOf(world::Direction::Right), ::Direction::RIGHT);
}

}  // namespace
