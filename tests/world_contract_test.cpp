#include "common/common_command.h"
#include "gtest/gtest.h"

namespace {


TEST(CommandContractTest, MoveCarriesOnlyTheDirection) {
    const protocol::ClientMessage message = protocol::MoveMessage{::Direction::RIGHT};
    const auto* move = std::get_if<protocol::MoveMessage>(&message);
    ASSERT_NE(move, nullptr);
    EXPECT_EQ(move->direction, ::Direction::RIGHT);
}

TEST(CommandContractTest, MoveHasItsOwnMessageType) {
    EXPECT_EQ(static_cast<uint8_t>(protocol::ClientMessageType::Move), 1);
}

}  // namespace
