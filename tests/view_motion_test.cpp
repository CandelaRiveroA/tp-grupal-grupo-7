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

ViewMotion makeMotion() { return ViewMotion(catalog()); }

constexpr int kCellPixels = 32;

const int kStepTicks =
	catalog().dataFor(world::CreatureKind::Player).cadence.stepTicks;

ViewEntity entityAt(int cellX, int cellY, int id = 0) {
	return ViewEntity{world::CreatureKind::Player, cellX, cellY,
			  world::Direction::Front, false,
			  world::ActionKind::Idle, std::nullopt, id};
}

LocalWorldView viewAt(int cellX, int cellY, int id = 0) {
	LocalWorldView view;
	view.entities.push_back(entityAt(cellX, cellY, id));
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
	view.entities.push_back(entityAt(5, 5, 0));
	view.entities.push_back(entityAt(2, 2, 1));
	ViewMotion motion = makeMotion();
	motion.observe(view, 0);

	LocalWorldView next;
	next.entities.push_back(entityAt(6, 5, 0));
	next.entities.push_back(entityAt(2, 2, 1));
	motion.observe(next, 100);

	const PixelOffset moving = motion.offsetFor(0, 100, kCellPixels);
	const PixelOffset still = motion.offsetFor(1, 100, kCellPixels);
	EXPECT_EQ(moving.x, -kCellPixels);
	EXPECT_EQ(still.x, 0);
	EXPECT_EQ(still.y, 0);

	EXPECT_EQ(motion.animationTickFor(0, 110), 10);
	EXPECT_EQ(motion.animationTickFor(1, 110), 110);
}

TEST(ViewMotionTest, ReorderedEntitiesKeepTheirOwnTrack) {
	// La lista de entidades puede cambiar de orden entre frames (el
	// servidor es libre de mandarla como quiera): el seguimiento es por
	// ID estable, no por posición, así que la interpolación de cada uno
	// sobrevive al reordenamiento.
	LocalWorldView view;
	view.entities.push_back(entityAt(5, 5, 0));
	view.entities.push_back(entityAt(2, 2, 1));
	ViewMotion motion = makeMotion();
	motion.observe(view, 0);

	LocalWorldView reordered;
	reordered.entities.push_back(entityAt(2, 2, 1));
	reordered.entities.push_back(entityAt(6, 5, 0));
	motion.observe(reordered, 100);

	// El de id 0 sigue cerrando su paso (5,5) -> (6,5), aunque ahora
	// sea el índice 1 de la lista.
	const PixelOffset player = motion.offsetFor(1, 100, kCellPixels);
	EXPECT_EQ(player.x, -kCellPixels);
	EXPECT_EQ(motion.animationTickFor(1, 110), 10);

	// El de id 1 nunca se movió: sigue plantado, aunque sea el índice 0.
	const PixelOffset still = motion.offsetFor(0, 100, kCellPixels);
	EXPECT_EQ(still.x, 0);
	EXPECT_EQ(still.y, 0);
}

TEST(ViewMotionTest, NewEntitiesDoNotInheritAnOldStep) {
	ViewMotion motion = makeMotion();
	motion.observe(viewAt(5, 5, 0), 0);
	motion.observe(viewAt(6, 5, 0), 100);

	LocalWorldView both;
	both.entities.push_back(entityAt(6, 5, 0));
	both.entities.push_back(entityAt(1, 1, 1));
	motion.observe(both, 110);

	// La entidad NUEVA (id 1) arranca sin historial: se dibuja en su
	// celda, con fase de animación propia desde esta iteración.
	const PixelOffset newcomer = motion.offsetFor(1, 110, kCellPixels);
	EXPECT_EQ(newcomer.x, 0);
	EXPECT_EQ(motion.animationTickFor(1, 110), 0);

	// La entidad conocida (id 0) NO pierde su paso por el cambio de
	// tamaño de la lista: sigue cerrando el paso aceptado.
	EXPECT_NE(motion.offsetFor(0, 110, kCellPixels).x, 0);
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

TEST(ViewMotionTest, DrawBoxUsesTheCellWhenThereIsNoHistory) {
	ViewMotion motion = makeMotion();
	const world::PixelRect box =
		motion.drawBoxFor(entityAt(5, 5), 0, 50, kCellPixels);

	EXPECT_EQ(box.x, 5 * kCellPixels);
	EXPECT_EQ(box.y, 5 * kCellPixels);
	EXPECT_EQ(box.width, kCellPixels);
	EXPECT_EQ(box.height, kCellPixels);
}

TEST(ViewMotionTest, DrawBoxStartsAtThePreviousCell) {
	ViewMotion motion = makeMotion();
	motion.observe(viewAt(5, 5), 0);
	motion.observe(viewAt(6, 5), 100);

	const world::PixelRect box =
		motion.drawBoxFor(entityAt(6, 5), 0, 100, kCellPixels);
	EXPECT_EQ(box.x, 5 * kCellPixels);
	EXPECT_EQ(box.y, 5 * kCellPixels);
}

TEST(ViewMotionTest, DrawBoxInterpolatesTheStep) {
	ViewMotion motion = makeMotion();
	motion.observe(viewAt(5, 5), 0);
	motion.observe(viewAt(6, 5), 100);

	const world::PixelRect box = motion.drawBoxFor(
		entityAt(6, 5), 0, 100 + kStepTicks / 2, kCellPixels);
	EXPECT_EQ(box.x, 5 * kCellPixels + kCellPixels / 2);
	EXPECT_EQ(box.y, 5 * kCellPixels);
}

TEST(ViewMotionTest, DrawBoxEndsAtTheCurrentCell) {
	ViewMotion motion = makeMotion();
	motion.observe(viewAt(5, 5), 0);
	motion.observe(viewAt(6, 5), 100);

	const world::PixelRect box =
		motion.drawBoxFor(entityAt(6, 5), 0, 100 + kStepTicks, kCellPixels);
	EXPECT_EQ(box.x, 6 * kCellPixels);
	EXPECT_EQ(box.y, 5 * kCellPixels);
}

TEST(ViewMotionTest, DrawBoxInterpolatesVerticalSteps) {
	ViewMotion motion = makeMotion();
	motion.observe(viewAt(5, 5), 0);
	motion.observe(viewAt(5, 4), 100);

	const world::PixelRect box =
		motion.drawBoxFor(entityAt(5, 4), 0, 100, kCellPixels);
	EXPECT_EQ(box.x, 5 * kCellPixels);
	EXPECT_EQ(box.y, 5 * kCellPixels);
}

TEST(ViewMotionTest, DrawBoxSnapsAfterATeleport) {
	ViewMotion motion = makeMotion();
	motion.observe(viewAt(5, 5), 0);
	motion.observe(viewAt(9, 9), 100);

	const world::PixelRect box =
		motion.drawBoxFor(entityAt(9, 9), 0, 100, kCellPixels);
	EXPECT_EQ(box.x, 9 * kCellPixels);
	EXPECT_EQ(box.y, 9 * kCellPixels);
}

TEST(ViewMotionTest, StepMidTransitContinuesFromDrawnPosition) {
	game::ViewMotion motion = makeMotion();

	// Paso de (0,0) a (1,0) arrancando en el tick 0.
	motion.observe(viewAt(0, 0), 0);
	motion.observe(viewAt(1, 0), 0);

	// A mitad de tránsito el sprite va entre ambas celdas.
	const int mid = kStepTicks / 2;
	const world::PixelRect origin =
		motion.drawBoxFor(entityAt(0, 0), 0, kStepTicks, kCellPixels);
	const world::PixelRect before =
		motion.drawBoxFor(entityAt(1, 0), 0, mid, kCellPixels);
	EXPECT_GT(before.x, origin.x);
	EXPECT_LT(before.x, origin.x + kCellPixels);

	// Llega un paso nuevo ANTES de cerrar la cadencia: el sprite
	// continúa desde donde estaba dibujado, sin saltar.
	motion.observe(viewAt(2, 0), mid);
	const world::PixelRect after =
		motion.drawBoxFor(entityAt(2, 0), 0, mid, kCellPixels);
	EXPECT_NEAR(after.x, before.x, 1);
	EXPECT_NEAR(after.y, before.y, 1);
}

TEST(ViewMotionTest, StepAfterCadenceStartsFromPreviousCell) {
	game::ViewMotion motion = makeMotion();

	// Paso de (0,0) a (1,0) completado en el tick kStepTicks.
	motion.observe(viewAt(0, 0), 0);
	motion.observe(viewAt(1, 0), 0);
	motion.observe(viewAt(1, 0), kStepTicks);

	// Al cerrar la ventana el sprite está exactamente en (1,0), y el
	// paso siguiente arranca desde ahí (comportamiento de siempre).
	const world::PixelRect done =
		motion.drawBoxFor(entityAt(1, 0), 0, kStepTicks, kCellPixels);
	motion.observe(viewAt(2, 0), kStepTicks);
	const world::PixelRect next =
		motion.drawBoxFor(entityAt(2, 0), 0, kStepTicks, kCellPixels);
	EXPECT_EQ(next.x, done.x);
	EXPECT_EQ(next.y, done.y);
}

}
