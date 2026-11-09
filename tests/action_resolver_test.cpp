#include "gtest/gtest.h"

#include "game/ActionResolver.h"
#include "game/LocalWorldView.h"
#include "game/ViewMotion.h"
#include "world/CharacterCatalog.h"
#include "world/CharacterCatalogLoader.h"
#include "world/CreatureCatalog.h"
#include "world/EntityAppearance.h"

namespace {

using game::ActionResolver;
using game::CarrySelection;
using game::LocalWorldView;
using game::ViewEntity;
using game::ViewMotion;
using world::ActionKind;

// Sin rasgos de contenido, nunca se sostiene nada: el resolver de estas
// pruebas sigue solo lo que dice la fuente de estado.
const world::ContentTraits kEmptyTraits;
const CarrySelection kNoCarry(kEmptyTraits);

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
    [characters.player.right.pickup]
    loops  = false
    frames = [
      [[4, 60], [10, 61]],
      [[5, 60], [11, 61]],
      [[0, 62], [6, 62]],
      [[1, 62], [7, 62]],
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

const world::CharacterCatalog& catalog() {
	static const world::CharacterCatalog instance =
		world::CharacterCatalogLoader().loadFromString(kPlayerToml);
	return instance;
}

const int kStepTicks =
	catalog().dataFor(world::CreatureKind::Player).cadence.stepTicks;
constexpr int kPlayerIndex = 0;

ViewEntity entity(ActionKind action = ActionKind::Idle) {
	ViewEntity e{world::CreatureKind::Player, 5, 5,
		     world::Direction::Front, false,
		     ActionKind::Idle, std::nullopt};
	e.action = action;
	return e;
}

LocalWorldView viewWith(const ViewEntity& first) {
	LocalWorldView view;
	view.entities.push_back(first);
	return view;
}


// La fuente (mock/servidor) decide la acción. El ActionResolver la sigue:
// Walk si la presentación detecta caminata y la acción es de locomoción;
// si no, la acción de la vista (las de la fuente no las pisa la caminata).

TEST(ActionResolverTest, QuietEntityDrawsTheViewAction) {
	ViewMotion motion(catalog());
	motion.observe(viewWith(entity()), 0);
	const ActionResolver resolver(motion, kNoCarry);

	EXPECT_EQ(resolver.actionFor(entity(), kPlayerIndex, 100),
		  ActionKind::Idle);
	EXPECT_EQ(resolver.actionFor(entity(ActionKind::Walk), kPlayerIndex,
				     100),
		  ActionKind::Walk);
}

TEST(ActionResolverTest, FollowsUseToolFromTheSource) {
	ViewMotion motion(catalog());
	// La fuente reporta UseTool: el resolver lo sigue.
	motion.observe(viewWith(entity(ActionKind::UseTool)), 100);
	const ActionResolver resolver(motion, kNoCarry);
	EXPECT_EQ(resolver.actionFor(entity(ActionKind::UseTool), kPlayerIndex,
				     100),
		  ActionKind::UseTool);
}

TEST(ActionResolverTest, BackToIdleWhenTheSourceStopsUseTool) {
	ViewMotion motion(catalog());
	motion.observe(viewWith(entity(ActionKind::UseTool)), 100);
	// La fuente vuelve a Idle: el resolver deja de mostrar UseTool.
	motion.observe(viewWith(entity(ActionKind::Idle)), 130);
	const ActionResolver resolver(motion, kNoCarry);
	EXPECT_EQ(resolver.actionFor(entity(ActionKind::Idle), kPlayerIndex,
				     130),
		  ActionKind::Idle);
}

TEST(ActionResolverTest, WateringCanUseToolAnimatesAsWater) {
	ViewMotion motion(catalog());
	ViewEntity watering = entity(ActionKind::UseTool);
	watering.activeTool = world::ItemKind::WateringCan;
	motion.observe(viewWith(watering), 100);
	const ActionResolver resolver(motion, kNoCarry);

	EXPECT_EQ(resolver.actionFor(watering, kPlayerIndex, 100),
		  ActionKind::Water);
}

TEST(ActionResolverTest, ToolWithoutRowKeepsUseTool) {
	ViewMotion motion(catalog());
	ViewEntity picking = entity(ActionKind::UseTool);
	picking.activeTool = world::ItemKind::Pickaxe;
	motion.observe(viewWith(picking), 100);
	const ActionResolver resolver(motion, kNoCarry);

	EXPECT_EQ(resolver.actionFor(picking, kPlayerIndex, 100),
		  ActionKind::UseTool);
}

TEST(ActionResolverTest, ConcreteActionFromTheSourceIsRespected) {
	ViewMotion motion(catalog());
	ViewEntity milking = entity(ActionKind::Milk);
	milking.activeTool = world::ItemKind::WateringCan;
	motion.observe(viewWith(milking), 100);
	const ActionResolver resolver(motion, kNoCarry);

	EXPECT_EQ(resolver.actionFor(milking, kPlayerIndex, 100),
		  ActionKind::Milk);
}

TEST(ActionResolverTest, SourceActionBeatsWalk) {
	ViewMotion motion(catalog());
	motion.observe(viewWith(entity()), 0);
	const ActionResolver resolver(motion, kNoCarry);

	// La caminata NO tapa a las acciones de la fuente: aunque la
	// entidad se esté moviendo, se anima lo que manda la fuente
	// (contrato §10).
	for (const ActionKind action :
	     {ActionKind::UseTool, ActionKind::Attack, ActionKind::Damage,
	      ActionKind::Death}) {
		ViewEntity moving = entity(action);
		moving.moving = true;
		EXPECT_EQ(resolver.actionFor(moving, kPlayerIndex, 100), action);
	}
}

TEST(ActionResolverTest, ToolPhaseStartsAtZeroWhenTheSourceBeginsUseTool) {
	ViewMotion motion(catalog());
	motion.observe(viewWith(entity(ActionKind::UseTool)), 100);
	const ActionResolver resolver(motion, kNoCarry);
	EXPECT_EQ(resolver.phaseFor(entity(ActionKind::UseTool), kPlayerIndex,
				    100),
		  0);
	EXPECT_EQ(resolver.phaseFor(entity(ActionKind::UseTool), kPlayerIndex,
				    112),
		  12);
}

TEST(ActionResolverTest, WalkKeepsTheStepAnchoredPhase) {
	ViewMotion motion(catalog());
	motion.observe(viewWith(entity()), 0);
	motion.observe(viewWith(ViewEntity{world::CreatureKind::Player, 6, 5,
					   world::Direction::Front, true,
					   ActionKind::Idle, std::nullopt}),
		       100);
	ViewEntity moving = entity();
	moving.moving = true;
	const ActionResolver resolver(motion, kNoCarry);

	EXPECT_EQ(resolver.actionFor(moving, kPlayerIndex, 105),
		  ActionKind::Walk);
	EXPECT_EQ(resolver.phaseFor(moving, kPlayerIndex, 105), 5);
}

TEST(ActionResolverTest, ToolHitPlaysEveryFrameInOrder) {
	ViewMotion motion(catalog());
	motion.observe(viewWith(entity(ActionKind::UseTool)), 100);
	const ActionResolver resolver(motion, kNoCarry);

	const world::CreatureData& creature =
		catalog().dataFor(world::CreatureKind::Player);
	const int frameTicks = creature.cadence.frameTicks;
	constexpr int kFrames = 5;
	for (int frame = 0; frame < kFrames; ++frame) {
		const int it = 100 + frame * frameTicks;
		const ViewEntity e = entity(ActionKind::UseTool);
		ASSERT_EQ(resolver.actionFor(e, kPlayerIndex, it),
			  ActionKind::UseTool)
			<< "en el cuadro " << frame;
		const int phase = resolver.phaseFor(e, kPlayerIndex, it);
		EXPECT_EQ(world::AppearanceResolver(creature).frameIndexFor(world::Direction::Right,
						ActionKind::UseTool, phase),
			  frame)
			<< "en el cuadro " << frame;
	}
}

TEST(ActionResolverTest, PickupAnimatesOnTheActionClock) {
	ViewMotion motion(catalog());
	// El jugador dejó de caminar hace rato (el reloj del paso quedó
	// viejo) y la fuente enciende Pickup: la fase corre desde el
	// arranque de la acción, así los cuadros salen 1-2-3-4.
	motion.observe(viewWith(entity()), 0);
	motion.observe(viewWith(entity(ActionKind::Pickup)), 100);
	const ActionResolver resolver(motion, kNoCarry);

	EXPECT_EQ(resolver.phaseFor(entity(ActionKind::Pickup), kPlayerIndex,
				    100),
		  0);
	EXPECT_EQ(resolver.phaseFor(entity(ActionKind::Pickup), kPlayerIndex,
				    124),
		  24);

	const world::CreatureData& creature =
		catalog().dataFor(world::CreatureKind::Player);
	const int frameTicks = creature.cadence.frameTicks;
	constexpr int kFrames = 4;
	for (int frame = 0; frame < kFrames; ++frame) {
		const int it = 100 + frame * frameTicks;
		const int phase = resolver.phaseFor(entity(ActionKind::Pickup),
						    kPlayerIndex, it);
		ASSERT_EQ(resolver.actionFor(entity(ActionKind::Pickup),
					     kPlayerIndex, it),
			  ActionKind::Pickup)
			<< "en el cuadro " << frame;
		EXPECT_EQ(world::AppearanceResolver(creature).frameIndexFor(
				  world::Direction::Right, ActionKind::Pickup,
				  phase),
			  frame)
			<< "en el cuadro " << frame;
	}
}

}
