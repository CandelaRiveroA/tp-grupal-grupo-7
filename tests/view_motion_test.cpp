#include "gtest/gtest.h"

#include "game/ViewMotion.h"
#include "world/CharacterCatalog.h"
#include "world/CharacterCatalogLoader.h"
#include "world/CreatureCatalog.h"

namespace {

using game::LocalWorldView;
using game::PixelOffset;
using game::ViewEntity;
using game::ViewMotion;

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
)";

const world::CharacterCatalog& catalog() {
	static const world::CharacterCatalog instance =
		world::CharacterCatalogLoader().loadFromString(kPlayerToml);
	return instance;
}

ViewMotion makeMotion() { return ViewMotion(catalog()); }

constexpr int kCellPixels = 32;

const int kStepTicks =
	catalog().dataFor(world::CreatureKind::Player).cadence.stepTicks;

ViewEntity entityAt(int cellX, int cellY) {
	return ViewEntity{world::CreatureKind::Player, cellX, cellY,
			  protocol::Direction::Front, false,
			  world::ActionKind::Idle, std::nullopt};
}

LocalWorldView viewAt(int cellX, int cellY) {
	LocalWorldView view;
	view.entities.push_back(entityAt(cellX, cellY));
	return view;
}


TEST(ViewMotionTest, FirstSightIsDrawnInItsCell) {
	ViewMotion motion = makeMotion();
	motion.observe(viewAt(5, 5), 100);
	const PixelOffset offset = motion.offsetFor(0, 100, kCellPixels);
	EXPECT_EQ(offset.x, 0);
	EXPECT_EQ(offset.y, 0);
	EXPECT_EQ(motion.animationTickFor(0, 100), 0);
}

TEST(ViewMotionTest, StepSlidesFromThePreviousCell) {
	ViewMotion motion = makeMotion();
	motion.observe(viewAt(5, 5), 0);
	motion.observe(viewAt(6, 5), 100);

	const PixelOffset start = motion.offsetFor(0, 100, kCellPixels);
	EXPECT_EQ(start.x, -kCellPixels);
	EXPECT_EQ(start.y, 0);

	const PixelOffset middle =
		motion.offsetFor(0, 100 + kStepTicks / 2, kCellPixels);
	EXPECT_EQ(middle.x, -kCellPixels / 2);
	EXPECT_EQ(middle.y, 0);

	const PixelOffset end =
		motion.offsetFor(0, 100 + kStepTicks, kCellPixels);
	EXPECT_EQ(end.x, 0);
	EXPECT_EQ(end.y, 0);
}

TEST(ViewMotionTest, OffsetEndsWhenIterationsAreLost) {
	ViewMotion motion = makeMotion();
	motion.observe(viewAt(5, 5), 0);
	motion.observe(viewAt(6, 5), 100);
	const PixelOffset late =
		motion.offsetFor(0, 100 + kStepTicks * 10, kCellPixels);
	EXPECT_EQ(late.x, 0);
	EXPECT_EQ(late.y, 0);
}

TEST(ViewMotionTest, VerticalStepInterpolatesOnTheYAxis) {
	ViewMotion motion = makeMotion();
	motion.observe(viewAt(5, 5), 0);
	motion.observe(viewAt(5, 4), 100);
	const PixelOffset start = motion.offsetFor(0, 100, kCellPixels);
	EXPECT_EQ(start.x, 0);
	EXPECT_EQ(start.y, kCellPixels);
}

TEST(ViewMotionTest, BlockedEntityStandsStill) {
	ViewMotion motion = makeMotion();
	motion.observe(viewAt(5, 5), 0);
	motion.observe(viewAt(5, 5), 100);
	motion.observe(viewAt(5, 5), 1000);
	const PixelOffset offset = motion.offsetFor(0, 1000, kCellPixels);
	EXPECT_EQ(offset.x, 0);
	EXPECT_EQ(offset.y, 0);
}

TEST(ViewMotionTest, TeleportSnapsWithoutSliding) {
	ViewMotion motion = makeMotion();
	motion.observe(viewAt(5, 5), 0);
	motion.observe(viewAt(9, 9), 100);
	const PixelOffset offset = motion.offsetFor(0, 100, kCellPixels);
	EXPECT_EQ(offset.x, 0);
	EXPECT_EQ(offset.y, 0);
}

TEST(ViewMotionTest, AnimationPhaseRestartsAtEveryObservedStep) {
	ViewMotion motion = makeMotion();
	motion.observe(viewAt(5, 5), 0);
	motion.observe(viewAt(6, 5), 100);
	EXPECT_EQ(motion.animationTickFor(0, 100), 0);
	EXPECT_EQ(motion.animationTickFor(0, 110), 10);

	motion.observe(viewAt(7, 5), 150);
	EXPECT_EQ(motion.animationTickFor(0, 150), 0);
	EXPECT_EQ(motion.animationTickFor(0, 160), 10);
}

TEST(ViewMotionTest, EachEntityKeepsItsOwnStep) {
	LocalWorldView view;
	view.entities.push_back(entityAt(5, 5));
	view.entities.push_back(entityAt(2, 2));
	ViewMotion motion = makeMotion();
	motion.observe(view, 0);

	LocalWorldView next;
	next.entities.push_back(entityAt(6, 5));
	next.entities.push_back(entityAt(2, 2));
	motion.observe(next, 100);

	const PixelOffset moving = motion.offsetFor(0, 100, kCellPixels);
	const PixelOffset still = motion.offsetFor(1, 100, kCellPixels);
	EXPECT_EQ(moving.x, -kCellPixels);
	EXPECT_EQ(still.x, 0);
	EXPECT_EQ(still.y, 0);

	EXPECT_EQ(motion.animationTickFor(0, 110), 10);
	EXPECT_EQ(motion.animationTickFor(1, 110), 110);
}

TEST(ViewMotionTest, NewEntitiesDoNotInheritAnOldStep) {
	ViewMotion motion = makeMotion();
	motion.observe(viewAt(5, 5), 0);
	motion.observe(viewAt(6, 5), 100);

	LocalWorldView both;
	both.entities.push_back(entityAt(6, 5));
	both.entities.push_back(entityAt(1, 1));
	motion.observe(both, 110);

	const PixelOffset first = motion.offsetFor(0, 110, kCellPixels);
	const PixelOffset second = motion.offsetFor(1, 110, kCellPixels);
	EXPECT_EQ(first.x, 0);
	EXPECT_EQ(second.x, 0);
}


TEST(ViewMotionTest, ReleasingTheKeyClosesTheStepBeforeGoingIdle) {
	ViewMotion motion = makeMotion();
	motion.observe(viewAt(5, 5), 0);
	motion.observe(viewAt(6, 5), 100);

	EXPECT_TRUE(motion.isWalking(0, 101, false));
	EXPECT_TRUE(motion.isWalking(0, 100 + kStepTicks - 1, false));

	EXPECT_FALSE(motion.isWalking(0, 100 + kStepTicks, false));
	EXPECT_FALSE(motion.isWalking(0, 100 + kStepTicks * 10, false));
}

TEST(ViewMotionTest, ServerMovingKeepsWalkingAfterTheStepCloses) {
	ViewMotion motion = makeMotion();
	motion.observe(viewAt(5, 5), 0);
	motion.observe(viewAt(6, 5), 100);
	EXPECT_TRUE(motion.isWalking(0, 100 + kStepTicks, true));
	EXPECT_TRUE(motion.isWalking(0, 100 + kStepTicks * 10, true));
}

TEST(ViewMotionTest, WithoutAStepThereIsNothingToClose) {
	ViewMotion first = makeMotion();
	first.observe(viewAt(5, 5), 100);
	EXPECT_FALSE(first.isWalking(0, 101, false));

	ViewMotion blocked = makeMotion();
	blocked.observe(viewAt(5, 5), 0);
	blocked.observe(viewAt(5, 5), 100);
	EXPECT_FALSE(blocked.isWalking(0, 101, false));

	ViewMotion teleport = makeMotion();
	teleport.observe(viewAt(5, 5), 0);
	teleport.observe(viewAt(9, 9), 100);
	EXPECT_FALSE(teleport.isWalking(0, 101, false));
}

TEST(ViewMotionTest, WithoutHistoryItDrawsInItsCell) {
	ViewMotion motion = makeMotion();
	const PixelOffset offset = motion.offsetFor(0, 50, kCellPixels);
	EXPECT_EQ(offset.x, 0);
	EXPECT_EQ(offset.y, 0);
	EXPECT_EQ(motion.animationTickFor(0, 50), 50);
}

}
