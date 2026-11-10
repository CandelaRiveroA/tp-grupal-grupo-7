#include <string>
#include <vector>

#include "gtest/gtest.h"

#include "world/CharacterCatalog.h"
#include "world/CharacterCatalogLoader.h"
#include "world/CreatureCatalog.h"
#include "world/EntityAppearance.h"

namespace {

using world::ActionKind;
using world::Direction;
using world::EntityAppearance;

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
    [characters.player.right.use_tool]
    loops  = false
    frame  = 8
    frames = [
      [[0, 16, 1, 2], [6, 16, 1, 2]],
      [[1, 16, 1, 2], [7, 16, 1, 2]],
      [[2, 16, 1, 2], [8, 16, 1, 2]],
      [[3, 16, 1, 2], [9, 16, 1, 2]],
      [[4, 16, 1, 2], [10, 16, 1, 2]],
    ]

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
)";

const world::CharacterCatalog& catalog() {
	static const world::CharacterCatalog instance =
		world::CharacterCatalogLoader().loadFromString(kPlayerToml);
	return instance;
}

const world::CreatureData& player() {
	return catalog().dataFor(world::CreatureKind::Player);
}

const world::DirectionAppearance& dir(Direction facing) {
	return player().directions[static_cast<int>(facing)];
}

const world::Pose* idlePoseOf(Direction facing) {
	return &dir(facing).actions.at(ActionKind::Idle).frames[0];
}
const world::Pose* walkPoseOf(Direction facing, std::size_t frame) {
	return &dir(facing).actions.at(ActionKind::Walk).frames[frame];
}

int ticks(int n) { return n * player().cadence.frameTicks; }

TEST(EntityAppearanceTest, IdleIgnoresTheAnimationTick) {
	const EntityAppearance start = world::AppearanceResolver(player()).appearanceFor( Direction::Front, ActionKind::Idle, 0);
	const EntityAppearance later = world::AppearanceResolver(player()).appearanceFor( Direction::Front, ActionKind::Idle, 999);
	EXPECT_EQ(start.pose, idlePoseOf(Direction::Front));
	EXPECT_EQ(later.pose, idlePoseOf(Direction::Front));
	EXPECT_FALSE(start.mirrored);
}

TEST(EntityAppearanceTest, WalkStartsOnTheFirstFrameOfTheSequence) {
	const EntityAppearance appearance = world::AppearanceResolver(player()).appearanceFor( Direction::Front, ActionKind::Walk, 0);
	EXPECT_EQ(appearance.pose, walkPoseOf(Direction::Front, 0));
}

TEST(EntityAppearanceTest, WalkAdvancesOneFrameEveryWalkFrameTicks) {
	const EntityAppearance second = world::AppearanceResolver(player()).appearanceFor( Direction::Front, ActionKind::Walk, ticks(1));
	EXPECT_EQ(second.pose, walkPoseOf(Direction::Front, 1));

	const EntityAppearance cycled = world::AppearanceResolver(player()).appearanceFor( Direction::Front, ActionKind::Walk, ticks(2));
	EXPECT_EQ(cycled.pose, walkPoseOf(Direction::Front, 0));
}

TEST(EntityAppearanceTest, FrameIsDecidedByTheIterationNotByRenderCalls) {
	const EntityAppearance dropped = world::AppearanceResolver(player()).appearanceFor( Direction::Front, ActionKind::Walk, ticks(3));
	const EntityAppearance again = world::AppearanceResolver(player()).appearanceFor( Direction::Front, ActionKind::Walk, ticks(3));
	EXPECT_EQ(dropped.pose, again.pose);
	EXPECT_EQ(dropped.pose, walkPoseOf(Direction::Front, 1));
}

TEST(EntityAppearanceTest, MirroredDirectionReusesTheSameFrames) {
	const EntityAppearance left = world::AppearanceResolver(player()).appearanceFor( Direction::Left, ActionKind::Walk, ticks(1));
	const EntityAppearance right = world::AppearanceResolver(player()).appearanceFor( Direction::Right, ActionKind::Walk, ticks(1));
	ASSERT_EQ(left.pose->size(), right.pose->size());
	for (std::size_t i = 0; i < left.pose->size(); ++i) {
		EXPECT_EQ((*left.pose)[i].frame.col,
			  (*right.pose)[i].frame.col);
		EXPECT_EQ((*left.pose)[i].frame.row,
			  (*right.pose)[i].frame.row);
	}
	EXPECT_TRUE(left.mirrored);
	EXPECT_FALSE(right.mirrored);
}


TEST(EntityAppearanceTest, WalkSheetCellStartsAtZeroAndCyclesTheSequence) {
	EXPECT_EQ(world::AppearanceResolver(player()).frameIndexFor( Direction::Front,
				       ActionKind::Walk, 0),
		  0);
	EXPECT_EQ(world::AppearanceResolver(player()).frameIndexFor( Direction::Front,
				       ActionKind::Walk, ticks(1)),
		  1);
	EXPECT_EQ(world::AppearanceResolver(player()).frameIndexFor( Direction::Front,
				       ActionKind::Walk, ticks(2)),
		  0);
	EXPECT_EQ(world::AppearanceResolver(player()).frameIndexFor( Direction::Front,
				       ActionKind::Walk, ticks(3)),
		  1);
}

TEST(EntityAppearanceTest, WalkSheetCellIsExactlyTheDrawnFrame) {
	const int stepTicks = player().cadence.stepTicks;
	for (int tick = 0; tick < 4 * stepTicks; tick += 7) {
		const int frame = world::AppearanceResolver(player()).frameIndexFor( Direction::Front, ActionKind::Walk, tick);
		const EntityAppearance appearance = world::AppearanceResolver(player()).appearanceFor( Direction::Front, ActionKind::Walk, tick);
		EXPECT_EQ(appearance.pose,
			  walkPoseOf(Direction::Front,
				     static_cast<std::size_t>(frame)))
			<< "en el tick " << tick;
	}
}


TEST(EntityAppearanceTest, IdleSheetCellIsAlwaysZero) {
	EXPECT_EQ(world::AppearanceResolver(player()).frameIndexFor( Direction::Front,
				       ActionKind::Idle, 0),
		  0);
	EXPECT_EQ(world::AppearanceResolver(player()).frameIndexFor( Direction::Front,
				       ActionKind::Idle, 999),
		  0);
}


TEST(EntityAppearanceTest, OneShotAdvancesWithItsOwnFrameTicks) {
	EXPECT_EQ(world::AppearanceResolver(player()).frameIndexFor( Direction::Right,
				       ActionKind::UseTool, 0),
		  0);
	EXPECT_EQ(world::AppearanceResolver(player()).frameIndexFor( Direction::Right,
				       ActionKind::UseTool, 7),
		  0);
	EXPECT_EQ(world::AppearanceResolver(player()).frameIndexFor( Direction::Right,
				       ActionKind::UseTool, 8),
		  1);
	EXPECT_EQ(world::AppearanceResolver(player()).frameIndexFor( Direction::Right,
				       ActionKind::UseTool, 16),
		  2);
	EXPECT_EQ(world::AppearanceResolver(player()).frameIndexFor( Direction::Right,
				       ActionKind::UseTool, 32),
		  4);
	EXPECT_EQ(world::AppearanceResolver(player()).frameIndexFor( Direction::Right,
				       ActionKind::UseTool, 80),
		  4);
}

TEST(EntityAppearanceTest, WalkKeepsTheCadenceFrameRateAlongsideAFastAction) {
	EXPECT_EQ(world::AppearanceResolver(player()).frameIndexFor( Direction::Right,
				       ActionKind::UseTool, 8),
		  1);
	EXPECT_EQ(world::AppearanceResolver(player()).frameIndexFor( Direction::Right,
				       ActionKind::Walk, 8),
		  0);
	EXPECT_EQ(world::AppearanceResolver(player()).frameIndexFor( Direction::Right,
				       ActionKind::Walk, 24),
		  0);
	EXPECT_EQ(world::AppearanceResolver(player()).frameIndexFor( Direction::Right,
				       ActionKind::UseTool, 24),
		  3);
}

TEST(EntityAppearanceTest, UncachedUseToolFallsBackToIdle) {
	const EntityAppearance front = world::AppearanceResolver(player()).appearanceFor( Direction::Front, ActionKind::UseTool, 0);
	const EntityAppearance back = world::AppearanceResolver(player()).appearanceFor( Direction::Back, ActionKind::UseTool, 0);
	EXPECT_EQ(front.pose, idlePoseOf(Direction::Front));
	EXPECT_EQ(back.pose, idlePoseOf(Direction::Back));
	EXPECT_FALSE(front.mirrored);
}

TEST(EntityAppearanceTest, UncachedActionSheetCellIsTheIdleFrame) {
	EXPECT_EQ(world::AppearanceResolver(player()).frameIndexFor( Direction::Front,
				       ActionKind::UseTool, 0),
		  0);
	EXPECT_EQ(world::AppearanceResolver(player()).frameIndexFor( Direction::Front,
				       ActionKind::UseTool, 999),
		  0);
}


TEST(EntityAppearanceTest, WalkCycleFitsExactlyOneStep) {
	EXPECT_GT(player().cadence.frameTicks, 0);
	for (const world::DirectionAppearance& direction : player().directions) {
		const world::ActionData& walk =
			direction.actions.at(ActionKind::Walk);
		EXPECT_EQ(player().cadence.stepTicks,
			  player().cadence.frameTicks *
				  static_cast<int>(walk.frames.size()));
	}
}

TEST(EntityAppearanceTest, AnotherCadenceChangesTheFrameRate) {
	std::string toml = kPlayerToml;
	const std::string from = "cadence = { step = 24, frame = 12 }";
	ASSERT_NE(toml.find(from), std::string::npos);
	toml.replace(toml.find(from), from.size(),
		     "cadence = { step = 40, frame = 20 }");
	const world::CharacterCatalog slowCatalog =
		world::CharacterCatalogLoader().loadFromString(toml);
	const world::CreatureData& slow =
		slowCatalog.dataFor(world::CreatureKind::Player);

	EXPECT_EQ(world::AppearanceResolver(player()).frameIndexFor( Direction::Front,
				       ActionKind::Walk, 12),
		  1);
	EXPECT_EQ(world::AppearanceResolver(slow).frameIndexFor(Direction::Front,
				       ActionKind::Walk, 12),
		  0);
	EXPECT_EQ(world::AppearanceResolver(slow).frameIndexFor(Direction::Front,
				       ActionKind::Walk, 20),
		  1);
}

}
