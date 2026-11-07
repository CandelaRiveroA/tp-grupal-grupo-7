#include "gtest/gtest.h"

#include "game/FootstepCue.h"
#include "world/CharacterCatalog.h"
#include "world/CharacterCatalogLoader.h"
#include "world/CreatureCatalog.h"
#include "world/EntityAppearance.h"

namespace {

using game::FootstepCue;
using world::Direction;

const char* const kPlayerToml = R"(
[characters.player]
sheet   = "walking.png"
grid    = [16, 32]
drawn   = [24, 48]
cadence = { step = 24, frame = 12 }
  [characters.player.front]
  mirrored = false
    [characters.player.front.idle]
    loops  = false
    frames = [ [[0, 0], [6, 0]] ]
    [characters.player.front.walk]
    loops  = true
    frames = [ [[1, 0], [7, 0]], [[2, 0], [8, 0]] ]
  [characters.player.back]
  mirrored = false
    [characters.player.back.idle]
    loops  = false
    frames = [ [[0, 2], [6, 2]] ]
    [characters.player.back.walk]
    loops  = true
    frames = [ [[1, 2], [7, 2]], [[2, 2], [8, 2]] ]
  [characters.player.left]
  mirrored = true
    [characters.player.left.idle]
    loops  = false
    frames = [ [[0, 1], [6, 1]] ]
    [characters.player.left.walk]
    loops  = true
    frames = [ [[1, 1], [7, 1]], [[2, 1], [8, 1]] ]
  [characters.player.right]
  mirrored = false
    [characters.player.right.idle]
    loops  = false
    frames = [ [[0, 1], [6, 1]] ]
    [characters.player.right.walk]
    loops  = true
    frames = [ [[1, 1], [7, 1]], [[2, 1], [8, 1]] ]

# Gallina mínima: el loader exige una tabla por criatura del enum.
# Walk de 1 cuadro => invariante step == frame * 1 (8 == 8 * 1).
[characters.chicken]
sheet   = "Chicken.png"
grid    = [16, 16]
drawn   = [32, 32]
cadence = { step = 8, frame = 8 }
  [characters.chicken.front]
  mirrored = false
    [characters.chicken.front.idle]
    loops  = false
    frames = [ [[0, 0]] ]
    [characters.chicken.front.walk]
    loops  = true
    frames = [ [[0, 0]] ]
  [characters.chicken.right]
  mirrored = false
    [characters.chicken.right.idle]
    loops  = false
    frames = [ [[0, 0]] ]
    [characters.chicken.right.walk]
    loops  = true
    frames = [ [[0, 0]] ]
  [characters.chicken.back]
  mirrored = false
    [characters.chicken.back.idle]
    loops  = false
    frames = [ [[0, 0]] ]
    [characters.chicken.back.walk]
    loops  = true
    frames = [ [[0, 0]] ]
  [characters.chicken.left]
  mirrored = false
    [characters.chicken.left.idle]
    loops  = false
    frames = [ [[0, 0]] ]
    [characters.chicken.left.walk]
    loops  = true
    frames = [ [[0, 0]] ]

# El loader exige una tabla por criatura del enum: vaca mínima.
[characters.cow]
sheet   = "Cow.png"
grid    = [32, 32]
drawn   = [64, 64]
cadence = { step = 8, frame = 8 }
  [characters.cow.front]
  mirrored = false
    [characters.cow.front.idle]
    loops  = false
    frames = [ [[0, 0]] ]
    [characters.cow.front.walk]
    loops  = true
    frames = [ [[0, 0]] ]
  [characters.cow.right]
  mirrored = false
    [characters.cow.right.idle]
    loops  = false
    frames = [ [[0, 0]] ]
    [characters.cow.right.walk]
    loops  = true
    frames = [ [[0, 0]] ]
  [characters.cow.back]
  mirrored = false
    [characters.cow.back.idle]
    loops  = false
    frames = [ [[0, 0]] ]
    [characters.cow.back.walk]
    loops  = true
    frames = [ [[0, 0]] ]
  [characters.cow.left]
  mirrored = false
    [characters.cow.left.idle]
    loops  = false
    frames = [ [[0, 0]] ]
    [characters.cow.left.walk]
    loops  = true
    frames = [ [[0, 0]] ]

[characters.orc]
sheet   = "Cow.png"
grid    = [32, 32]
drawn   = [64, 64]
cadence = { step = 8, frame = 8 }
  [characters.orc.front]
  mirrored = false
    [characters.orc.front.idle]
    loops  = false
    frames = [ [[0, 0]] ]
    [characters.orc.front.walk]
    loops  = true
    frames = [ [[0, 0]] ]
  [characters.orc.right]
  mirrored = false
    [characters.orc.right.idle]
    loops  = false
    frames = [ [[0, 0]] ]
    [characters.orc.right.walk]
    loops  = true
    frames = [ [[0, 0]] ]
  [characters.orc.back]
  mirrored = false
    [characters.orc.back.idle]
    loops  = false
    frames = [ [[0, 0]] ]
    [characters.orc.back.walk]
    loops  = true
    frames = [ [[0, 0]] ]
  [characters.orc.left]
  mirrored = false
    [characters.orc.left.idle]
    loops  = false
    frames = [ [[0, 0]] ]
    [characters.orc.left.walk]
    loops  = true
    frames = [ [[0, 0]] ]

[characters.skeleton]
sheet   = "Cow.png"
grid    = [32, 32]
drawn   = [64, 64]
cadence = { step = 8, frame = 8 }
  [characters.skeleton.front]
  mirrored = false
    [characters.skeleton.front.idle]
    loops  = false
    frames = [ [[0, 0]] ]
    [characters.skeleton.front.walk]
    loops  = true
    frames = [ [[0, 0]] ]
  [characters.skeleton.right]
  mirrored = false
    [characters.skeleton.right.idle]
    loops  = false
    frames = [ [[0, 0]] ]
    [characters.skeleton.right.walk]
    loops  = true
    frames = [ [[0, 0]] ]
  [characters.skeleton.back]
  mirrored = false
    [characters.skeleton.back.idle]
    loops  = false
    frames = [ [[0, 0]] ]
    [characters.skeleton.back.walk]
    loops  = true
    frames = [ [[0, 0]] ]
  [characters.skeleton.left]
  mirrored = false
    [characters.skeleton.left.idle]
    loops  = false
    frames = [ [[0, 0]] ]
    [characters.skeleton.left.walk]
    loops  = true
    frames = [ [[0, 0]] ]

[characters.vampire]
sheet   = "Cow.png"
grid    = [32, 32]
drawn   = [64, 64]
cadence = { step = 8, frame = 8 }
  [characters.vampire.front]
  mirrored = false
    [characters.vampire.front.idle]
    loops  = false
    frames = [ [[0, 0]] ]
    [characters.vampire.front.walk]
    loops  = true
    frames = [ [[0, 0]] ]
  [characters.vampire.right]
  mirrored = false
    [characters.vampire.right.idle]
    loops  = false
    frames = [ [[0, 0]] ]
    [characters.vampire.right.walk]
    loops  = true
    frames = [ [[0, 0]] ]
  [characters.vampire.back]
  mirrored = false
    [characters.vampire.back.idle]
    loops  = false
    frames = [ [[0, 0]] ]
    [characters.vampire.back.walk]
    loops  = true
    frames = [ [[0, 0]] ]
  [characters.vampire.left]
  mirrored = false
    [characters.vampire.left.idle]
    loops  = false
    frames = [ [[0, 0]] ]
    [characters.vampire.left.walk]
    loops  = true
    frames = [ [[0, 0]] ]
)";

const world::CreatureData& playerData() {
	static const world::CharacterCatalog catalog =
		world::CharacterCatalogLoader().loadFromString(kPlayerToml);
	return catalog.dataFor(world::CreatureKind::Player);
}


TEST(FootstepCueTest, StandingStillNeverTriggers) {
	FootstepCue cue;
	EXPECT_FALSE(cue.update(false, 0));
	EXPECT_FALSE(cue.update(false, 1));
	EXPECT_FALSE(cue.update(false, 0));
}

TEST(FootstepCueTest, StartingToWalkTriggersExactlyOnce) {
	FootstepCue cue;
	EXPECT_TRUE(cue.update(true, 0));
	EXPECT_FALSE(cue.update(true, 0));
}

TEST(FootstepCueTest, SameFrameTriggersNothing) {
	FootstepCue cue;
	EXPECT_TRUE(cue.update(true, 1));
	EXPECT_FALSE(cue.update(true, 1));
	EXPECT_FALSE(cue.update(true, 1));
}

TEST(FootstepCueTest, EveryAssetChangeTriggers) {
	FootstepCue cue;
	EXPECT_TRUE(cue.update(true, 0));
	EXPECT_TRUE(cue.update(true, 1));
	EXPECT_TRUE(cue.update(true, 0));
	EXPECT_TRUE(cue.update(true, 1));
}

TEST(FootstepCueTest, StoppingDoesNotTrigger) {
	FootstepCue cue;
	EXPECT_TRUE(cue.update(true, 0));
	EXPECT_FALSE(cue.update(false, 0));
	EXPECT_FALSE(cue.update(false, 1));
}

TEST(FootstepCueTest, WalkingAgainTriggersEvenWithTheSameFrameNumber) {
	FootstepCue cue;
	EXPECT_TRUE(cue.update(true, 0));
	EXPECT_FALSE(cue.update(false, 0));
	EXPECT_TRUE(cue.update(true, 0));
}

TEST(FootstepCueTest, OneTriggerPerAssetChangeDuringAWholeStep) {
	const world::CreatureData& creature = playerData();

	FootstepCue cue;
	int triggers = 0;
	for (int it = 0; it < creature.cadence.stepTicks; ++it) {
		const int walkFrame = world::AppearanceResolver(creature).frameIndexFor( Direction::Front, world::ActionKind::Walk,
			it);
		if (cue.update(true, walkFrame)) {
			++triggers;
		}
	}
	EXPECT_EQ(triggers, 2);
}

}
