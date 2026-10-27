#include "gtest/gtest.h"

#include "game/SoundGate.h"

namespace {

TEST(SoundGateTest, AllowsCueTheRoomDeclares) {
	const game::SoundGate gate({"chicken_cluck", "rooster"});
	EXPECT_TRUE(gate.allows(game::CueKind::ChickenCluck));
}

TEST(SoundGateTest, BlocksCueTheRoomDoesNotDeclare) {
	// Una sala que no declara el cue lo silencia: decisión por
	// datos, no error.
	const game::SoundGate gate({"rooster"});
	EXPECT_FALSE(gate.allows(game::CueKind::ChickenCluck));
}

TEST(SoundGateTest, RoomWithoutSoundsIsSilent) {
	const game::SoundGate gate({});
	EXPECT_FALSE(gate.allows(game::CueKind::ChickenCluck));
}

}
