#include "gtest/gtest.h"

#include "game/ActionResolver.h"
#include "game/LocalWorldView.h"
#include "game/ToolUse.h"
#include "game/ViewMotion.h"
#include "world/CharacterCatalog.h"
#include "world/CharacterCatalogLoader.h"
#include "world/CreatureCatalog.h"
#include "world/EntityAppearance.h"

namespace {

using game::ActionResolver;
using game::LocalWorldView;
using game::ToolUse;
using game::ViewEntity;
using game::ViewMotion;
using world::ActionKind;

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

const int kStepTicks =
	catalog().dataFor(world::CreatureKind::Player).cadence.stepTicks;
constexpr int kGolpeTicks = 60;
constexpr int kPlayerIndex = 0;
constexpr int kOtherIndex = 1;

ViewEntity entity(ActionKind action = ActionKind::Idle) {
	ViewEntity entity{world::CreatureKind::Player, 5, 5,
			  protocol::Direction::Front, false};
	entity.action = action;
	return entity;
}

LocalWorldView viewWith(const ViewEntity& first) {
	LocalWorldView view;
	view.entities.push_back(first);
	return view;
}

ViewMotion motionStanding() {
	ViewMotion motion(catalog());
	motion.observe(viewWith(entity()), 0);
	return motion;
}


TEST(ActionResolverTest, QuietEntityDrawsTheViewAction) {
	const ViewMotion motion = motionStanding();
	ToolUse toolUse(kGolpeTicks);
	const ActionResolver resolver(motion, toolUse, kPlayerIndex);
	EXPECT_EQ(resolver.actionFor(entity(), kPlayerIndex, 100),
		  ActionKind::Idle);
	EXPECT_EQ(resolver.actionFor(entity(ActionKind::Walk), kPlayerIndex,
				     100),
		  ActionKind::Walk);
}

TEST(ActionResolverTest, ToolWindowDrawsUseTool) {
	const ViewMotion motion = motionStanding();
	ToolUse toolUse(kGolpeTicks);
	const ActionResolver resolver(motion, toolUse, kPlayerIndex);

	EXPECT_EQ(resolver.actionFor(entity(), kPlayerIndex, 100),
		  ActionKind::Idle);
	toolUse.trigger(100);
	EXPECT_EQ(resolver.actionFor(entity(), kPlayerIndex, 100),
		  ActionKind::UseTool);
	EXPECT_EQ(resolver.actionFor(entity(), kPlayerIndex, 100 + 30),
		  ActionKind::UseTool);
	EXPECT_EQ(resolver.actionFor(entity(), kPlayerIndex, 100 + 60 - 1),
		  ActionKind::UseTool);
}

TEST(ActionResolverTest, AfterTheToolWindowItGoesBackToTheViewAction) {
	const ViewMotion motion = motionStanding();
	ToolUse toolUse(kGolpeTicks);
	const ActionResolver resolver(motion, toolUse, kPlayerIndex);
	toolUse.trigger(100);
	EXPECT_EQ(resolver.actionFor(entity(), kPlayerIndex, 100 + 60),
		  ActionKind::Idle);
}

TEST(ActionResolverTest, WalkingBeatsTheToolHit) {
	const ViewMotion motion = motionStanding();
	ViewEntity moving = entity();
	moving.moving = true;
	ToolUse toolUse(kGolpeTicks);
	const ActionResolver resolver(motion, toolUse, kPlayerIndex);
	toolUse.trigger(100);

	EXPECT_EQ(resolver.actionFor(moving, kPlayerIndex, 100),
		  ActionKind::Walk);
}

TEST(ActionResolverTest, ClosingTheStepBeatsTheToolHit) {
	ViewMotion motion(catalog());
	motion.observe(viewWith(entity()), 0);
	const ViewEntity standing = entity();
	motion.observe(viewWith(ViewEntity{world::CreatureKind::Player, 6, 5,
					   protocol::Direction::Front, true}),
		       kStepTicks);
	ToolUse toolUse(kGolpeTicks);
	const ActionResolver resolver(motion, toolUse, kPlayerIndex);
	toolUse.trigger(kStepTicks);

	EXPECT_EQ(resolver.actionFor(standing, kPlayerIndex, kStepTicks + 1),
		  ActionKind::Walk);
	EXPECT_EQ(
		resolver.actionFor(standing, kPlayerIndex,
				   kStepTicks + kStepTicks),
		ActionKind::UseTool);
	EXPECT_EQ(
		resolver.actionFor(standing, kPlayerIndex,
				   kStepTicks + kStepTicks + kGolpeTicks),
		ActionKind::Idle);
}

TEST(ActionResolverTest, TheToolHitBelongsToTheLocalPlayerOnly) {
	const ViewMotion motion = motionStanding();
	ToolUse toolUse(kGolpeTicks);
	const ActionResolver resolver(motion, toolUse, kPlayerIndex);
	toolUse.trigger(100);
	EXPECT_EQ(resolver.actionFor(entity(), kPlayerIndex, 100),
		  ActionKind::UseTool);
	EXPECT_EQ(resolver.actionFor(entity(), kOtherIndex, 100),
		  ActionKind::Idle);
}


TEST(ActionResolverTest, ToolPhaseStartsAtZeroOnTheTrigger) {
	const ViewMotion motion = motionStanding();
	ToolUse toolUse(kGolpeTicks);
	const ActionResolver resolver(motion, toolUse, kPlayerIndex);
	toolUse.trigger(100);
	EXPECT_EQ(resolver.phaseFor(entity(), kPlayerIndex, 100), 0);
	EXPECT_EQ(resolver.phaseFor(entity(), kPlayerIndex, 100 + 12), 12);
	EXPECT_EQ(resolver.phaseFor(entity(), kPlayerIndex, 100 + 59), 59);
}

TEST(ActionResolverTest, ToolPhaseIgnoresHowLongThePlayerStoodStill) {
	const ViewMotion motion = motionStanding();
	ToolUse toolUse(kGolpeTicks);
	const ActionResolver resolver(motion, toolUse, kPlayerIndex);
	toolUse.trigger(100000);

	EXPECT_EQ(resolver.phaseFor(entity(), kPlayerIndex, 100000), 0);
	EXPECT_EQ(resolver.phaseFor(entity(), kPlayerIndex, 100024), 24);
	EXPECT_EQ(motion.animationTickFor(kPlayerIndex, 100000), 100000);
}

TEST(ActionResolverTest, WalkKeepsTheStepAnchoredPhase) {
	ViewMotion motion(catalog());
	motion.observe(viewWith(entity()), 0);
	motion.observe(viewWith(ViewEntity{world::CreatureKind::Player, 6, 5,
					   protocol::Direction::Front, true}),
		       100);
	ViewEntity moving = entity();
	moving.moving = true;
	ToolUse toolUse(kGolpeTicks);
	const ActionResolver resolver(motion, toolUse, kPlayerIndex);

	EXPECT_EQ(resolver.actionFor(moving, kPlayerIndex, 105),
		  ActionKind::Walk);
	EXPECT_EQ(resolver.phaseFor(moving, kPlayerIndex, 105), 5);
}

TEST(ActionResolverTest, ToolHitPlaysEveryFrameInOrder) {
	const ViewMotion motion = motionStanding();
	ToolUse toolUse(kGolpeTicks);
	const ActionResolver resolver(motion, toolUse, kPlayerIndex);
	toolUse.trigger(100);

	const world::CreatureData& creature =
		catalog().dataFor(world::CreatureKind::Player);
	const int frameTicks = creature.cadence.frameTicks;
	constexpr int kFrames = 5;
	for (int frame = 0; frame < kFrames; ++frame) {
		const int it = 100 + frame * frameTicks;
		ASSERT_EQ(resolver.actionFor(entity(), kPlayerIndex, it),
			  ActionKind::UseTool)
			<< "en el cuadro " << frame;
		const int phase = resolver.phaseFor(entity(), kPlayerIndex, it);
		EXPECT_EQ(world::frameIndexFor(creature, world::Direction::Right,
						ActionKind::UseTool, phase),
			  frame)
			<< "en el cuadro " << frame;
	}
	EXPECT_EQ(resolver.actionFor(entity(), kPlayerIndex, 100 + kGolpeTicks),
		  ActionKind::Idle);
}

}
