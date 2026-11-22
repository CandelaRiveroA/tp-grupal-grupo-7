#include <optional>

#include "editor_world_fixtures.h"
#include "game/AreaProximity.h"
#include "game/LocalWorldView.h"
#include "game/StructureSelection.h"
#include "gtest/gtest.h"
#include "world/MapSceneAdapter.h"
#include "world/OverlayAudit.h"
#include "world/OverlayLoader.h"

namespace {

// Edificios del fixture "town" (celdas de 32 px con escala 2):
//   mercado 2x2 en (1,1), puerta (2,3)
//   centro comunitario 2x2 en (4,3), puerta (5,3)
constexpr int kCell = 32;

const char* const kRolesToml = R"(
[roles.market]
action = "enter_map"
target = "market"

[roles.community_center]
action = "popup"
target = "plaza"
)";

game::LocalWorldView viewWithPlayerAt(int cellX, int cellY) {
	game::LocalWorldView view;
	view.entities.push_back(game::ViewEntity{
		world::CreatureKind::Player, cellX, cellY,
		world::Direction::Front, false, world::ActionKind::Idle,
		std::nullopt});
	return view;
}

world::SceneDescription townWith(const worldmodel::World& world,
				 const char* overlayToml) {
	const world::WorldOverlay overlay =
		world::OverlayLoader().loadFromString(overlayToml);
	return world::MapSceneAdapter(overlay).adapt(world, "town");
}

world::PixelPoint centerOfCell(int column, int row) {
	return world::PixelPoint{column * kCell + kCell / 2,
				 row * kCell + kCell / 2};
}

TEST(StructureSceneTest, BuildingsBecomeStructuresWithTheirRoleBehavior) {
	const world::SceneDescription scene =
		townWith(editor_world_fixtures::makeWorld(), kRolesToml);
	// Árbol (decoración): no es estructura. Mercado y centro: sí.
	ASSERT_EQ(scene.structures.size(), 2u);
	const world::SceneStructure& market = scene.structures[0];
	EXPECT_EQ(market.role, "market");
	EXPECT_EQ(market.at.column, 1);
	EXPECT_EQ(market.span.columns, 2);
	ASSERT_TRUE(market.door.has_value());
	EXPECT_EQ(market.door->column, 2);
	EXPECT_EQ(market.door->row, 3);
	EXPECT_EQ(market.action, world::StructureAction::EnterMap);
	EXPECT_EQ(market.target, "market");
	// El elemento que la dibuja es el del mercado (segundo, tras el árbol).
	EXPECT_EQ(scene.elements[market.elementIndex].at.column, 1);

	const world::SceneStructure& center = scene.structures[1];
	EXPECT_EQ(center.role, "community_center");
	EXPECT_EQ(center.action, world::StructureAction::Popup);
	EXPECT_EQ(center.target, "plaza");
}

TEST(StructureSceneTest, RoleWithoutBehaviorHasNoAction) {
	const world::SceneDescription scene =
		townWith(editor_world_fixtures::makeWorld(), "");
	ASSERT_EQ(scene.structures.size(), 2u);
	EXPECT_EQ(scene.structures[0].action, world::StructureAction::None);
}

TEST(StructureSceneTest, StructureKeepsTheDayElementWhenItHasANightVariant) {
	const world::SceneDescription scene = townWith(
		editor_world_fixtures::makeWorld(), R"(
[decorations."prefab_b:2,0"]
night_tile = "prefab_b:2,5"

[roles.market]
action = "popup"
target = "x"
)");
	const world::SceneStructure& market = scene.structures[0];
	ASSERT_TRUE(scene.elements[market.elementIndex].when.has_value());
	EXPECT_EQ(*scene.elements[market.elementIndex].when,
		  world::TimeOfDay::Day);
}

TEST(StructureSelectionTest, OnlyStructuresWithABehaviorAreClickable) {
	const world::SceneDescription withRoles =
		townWith(editor_world_fixtures::makeWorld(), kRolesToml);
	const world::SceneDescription withoutRoles =
		townWith(editor_world_fixtures::makeWorld(), "");
	const game::StructureSelection active(withRoles, kCell);
	const game::StructureSelection inactive(withoutRoles, kCell);
	const game::LocalWorldView near = viewWithPlayerAt(1, 3);
	EXPECT_TRUE(active.clickAt(centerOfCell(1, 1), near).hit);
	EXPECT_FALSE(inactive.clickAt(centerOfCell(1, 1), near).hit);
}

TEST(StructureSelectionTest, ClickOutsideAnyBuildingIsNotAHit) {
	const world::SceneDescription scene =
		townWith(editor_world_fixtures::makeWorld(), kRolesToml);
	const game::StructureSelection selection(scene, kCell);
	EXPECT_FALSE(selection.clickAt(centerOfCell(0, 0), viewWithPlayerAt(1, 3)).hit);
}

TEST(StructureSelectionTest, ClickBoxIsExactlyTheDrawnCells) {
	const world::SceneDescription scene =
		townWith(editor_world_fixtures::makeWorld(), kRolesToml);
	const game::StructureSelection selection(scene, kCell);
	const game::LocalWorldView near = viewWithPlayerAt(1, 3);
	// El mercado ocupa las celdas (1,1)-(2,2): adentro de la primera y
	// la última celda cuenta; un píxel afuera no.
	EXPECT_TRUE(selection.clickAt({1 * kCell, 1 * kCell}, near).hit);
	EXPECT_TRUE(selection.clickAt({3 * kCell - 1, 3 * kCell - 1}, near).hit);
	EXPECT_FALSE(selection.clickAt({3 * kCell, 2 * kCell}, near).hit);
	EXPECT_FALSE(selection.clickAt({1 * kCell - 1, 1 * kCell}, near).hit);
}

TEST(StructureSelectionTest, ClickIsInRangeOnlyNearTheBuilding) {
	const world::SceneDescription scene =
		townWith(editor_world_fixtures::makeWorld(), kRolesToml);
	const game::StructureSelection selection(scene, kCell);
	const world::PixelPoint onMarket = centerOfCell(1, 1);

	// Pegado al mercado (anillo de 1 celda): alcanza.
	const game::StructureClick close =
		selection.clickAt(onMarket, viewWithPlayerAt(2, 3));
	EXPECT_TRUE(close.hit);
	EXPECT_TRUE(close.inRange);
	ASSERT_NE(close.structure, nullptr);
	EXPECT_EQ(close.structure->role, "market");

	// Lejos: el click se consume pero no hace nada.
	const game::StructureClick far =
		selection.clickAt(onMarket, viewWithPlayerAt(5, 0));
	EXPECT_TRUE(far.hit);
	EXPECT_FALSE(far.inRange);
}

TEST(StructureSelectionTest, CursorOnlyReactsWhenTheBuildingIsInRange) {
	const world::SceneDescription scene =
		townWith(editor_world_fixtures::makeWorld(), kRolesToml);
	const game::StructureSelection selection(scene, kCell);
	EXPECT_TRUE(selection.hits(centerOfCell(1, 1), viewWithPlayerAt(2, 3)));
	EXPECT_FALSE(selection.hits(centerOfCell(1, 1), viewWithPlayerAt(5, 0)));
	EXPECT_FALSE(selection.hits(centerOfCell(1, 1), game::LocalWorldView{}));
}

// Si el editor mueve el edificio, la caja de click lo acompaña.
TEST(StructureSelectionTest, MovingTheBuildingMovesItsClickBox) {
	worldmodel::World world = editor_world_fixtures::makeWorld();
	world.maps["town"].objects[1].cell = {3, 0};
	const world::SceneDescription scene = townWith(world, kRolesToml);
	const game::StructureSelection selection(scene, kCell);
	const game::LocalWorldView near = viewWithPlayerAt(3, 2);
	EXPECT_TRUE(selection.clickAt(centerOfCell(3, 0), near).hit);
	EXPECT_FALSE(selection.clickAt(centerOfCell(1, 1), near).hit);
}

TEST(StructureSelectionTest, ReindexReplacesThePreviousScene) {
	const world::SceneDescription scene =
		townWith(editor_world_fixtures::makeWorld(), kRolesToml);
	game::StructureSelection selection(scene, kCell);
	selection.reindex(world::SceneDescription{}, kCell);
	EXPECT_FALSE(selection.clickAt(centerOfCell(1, 1), viewWithPlayerAt(2, 3)).hit);
}

TEST(AreaProximityTest, InsideAndRingAreNear) {
	const game::AreaProximity proximity(1);
	const world::GridCell at{4, 4};
	const world::GridSpan span{2, 3};
	EXPECT_TRUE(proximity.around(at, span, 4, 4));
	EXPECT_TRUE(proximity.around(at, span, 5, 6));
	// El anillo de una celda alrededor.
	EXPECT_TRUE(proximity.around(at, span, 3, 3));
	EXPECT_TRUE(proximity.around(at, span, 6, 7));
	// Dos celdas afuera ya no.
	EXPECT_FALSE(proximity.around(at, span, 2, 4));
	EXPECT_FALSE(proximity.around(at, span, 7, 4));
	EXPECT_FALSE(proximity.around(at, span, 4, 8));
}

TEST(OverlayRolesTest, UnknownActionIsInvalidWithItsPath) {
	try {
		world::OverlayLoader().loadFromString(R"(
[roles.market]
action = "teleport"
target = "x"
)");
		FAIL() << "se esperaba ConfigError";
	} catch (const world::ConfigError& error) {
		EXPECT_EQ(error.kind(), world::ConfigErrorKind::InvalidValue);
		EXPECT_EQ(error.path(), "roles.market.action");
	}
}

TEST(OverlayRolesTest, EmptyTargetIsInvalid) {
	try {
		world::OverlayLoader().loadFromString(R"(
[roles.market]
action = "popup"
target = ""
)");
		FAIL() << "se esperaba ConfigError";
	} catch (const world::ConfigError& error) {
		EXPECT_EQ(error.path(), "roles.market.target");
	}
}

TEST(OverlayRolesTest, UnknownRoleIsReportedByTheAudit) {
	const world::WorldOverlay overlay = world::OverlayLoader().loadFromString(R"(
[roles.palacio]
action = "popup"
target = "x"
)");
	const std::vector<world::ConfigError> findings =
		world::OverlayAudit().unmatched(
			editor_world_fixtures::makeWorld(), overlay);
	ASSERT_EQ(findings.size(), 1u);
	EXPECT_EQ(findings[0].path(), "roles.palacio");
}

}  // namespace
