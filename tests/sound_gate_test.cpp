#include "gtest/gtest.h"

#include "game/SoundGate.h"

namespace {

TEST(SoundGateTest, AllowsCueTheRoomDeclares) {
	const game::SoundGate gate({"chicken_cluck", "rooster"});
	EXPECT_TRUE(gate.allows(game::CueKind::ChickenCluck));
}

TEST(SoundGateTest, AllowsEachCueItsRoomLists) {
	// La sala declara varios cues: cada uno suena si está en la lista.
	const game::SoundGate gate({"chicken_cluck", "cow_moo"});
	EXPECT_TRUE(gate.allows(game::CueKind::ChickenCluck));
	EXPECT_TRUE(gate.allows(game::CueKind::CowMoo));
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
