#include <cstdio>
#include <fstream>
#include <string>

#include "editor_world_fixtures.h"
#include "gtest/gtest.h"
#include "world/ConfigError.h"
#include "world/EditorWorldSource.h"
#include "world/ElementLayout.h"
#include "world/MapSceneAdapter.h"
#include "world/OverlayAudit.h"
#include "world/OverlayLoader.h"
#include "world/SceneSources.h"
#include "world/ScenePixelMapper.h"

namespace {

constexpr int kScale = 2;

world::WorldOverlay overlayWith(int scale) {
	world::WorldOverlay overlay;
	overlay.scale = scale;
	return overlay;
}

world::SceneDescription adaptTown() {
	return world::MapSceneAdapter(overlayWith(kScale)).adapt(
		editor_world_fixtures::makeWorld(), "town");
}

TEST(MapSceneAdapterTest, GridAndCellSizeComeFromTheMap) {
	const world::SceneDescription scene = adaptTown();
	EXPECT_EQ(scene.grid.columns, 6);
	EXPECT_EQ(scene.grid.rows, 5);
	EXPECT_EQ(scene.cellPixelSize, 16 * kScale);
}

TEST(MapSceneAdapterTest, TilesetsAreCopiedByName) {
	const world::SceneDescription scene = adaptTown();
	ASSERT_EQ(scene.tilesets.count("prefab_b"), 1u);
	EXPECT_EQ(scene.tilesets.at("prefab_b").image, "World/prefabs/b.png");
	EXPECT_EQ(scene.tilesets.at("prefab_b").cellWidth, 16);
}

TEST(MapSceneAdapterTest, EveryObjectBecomesAnElementInFileOrder) {
	const world::SceneDescription scene = adaptTown();
	ASSERT_EQ(scene.elements.size(), 3u);
	// El árbol (1x2) en (4,0), luego el mercado y el centro (2x2).
	EXPECT_EQ(scene.elements[0].at.column, 4);
	EXPECT_EQ(scene.elements[0].at.row, 0);
	EXPECT_EQ(scene.elements[1].at.column, 1);
	EXPECT_EQ(scene.elements[2].at.column, 4);
	EXPECT_EQ(scene.elements[2].at.row, 3);
}

TEST(MapSceneAdapterTest, SpanIsTheObjectSizeInCells) {
	const world::SceneDescription scene = adaptTown();
	EXPECT_EQ(scene.elements[0].span.columns, 1);
	EXPECT_EQ(scene.elements[0].span.rows, 2);
	EXPECT_EQ(scene.elements[1].span.columns, 2);
	EXPECT_EQ(scene.elements[1].span.rows, 2);
}

TEST(MapSceneAdapterTest, PieceIsTheContiguousBlockFromTheOriginTile) {
	const world::SceneDescription scene = adaptTown();
	const world::ElementPlacement& market = scene.elements[1];
	ASSERT_EQ(market.pieceSize.columns, 2);
	ASSERT_EQ(market.pieceSize.rows, 2);
	ASSERT_EQ(market.pieceCells.size(), 4u);
	// Origen prefab_b:2,0 -> (2,0)(3,0) / (2,1)(3,1), fila por fila.
	EXPECT_EQ(market.pieceCells[0].column, 2);
	EXPECT_EQ(market.pieceCells[0].row, 0);
	EXPECT_EQ(market.pieceCells[1].column, 3);
	EXPECT_EQ(market.pieceCells[1].row, 0);
	EXPECT_EQ(market.pieceCells[2].column, 2);
	EXPECT_EQ(market.pieceCells[2].row, 1);
	EXPECT_EQ(market.pieceCells[3].column, 3);
	EXPECT_EQ(market.pieceCells[3].row, 1);
}

TEST(MapSceneAdapterTest, EditorObjectsAreSortedInDepthWithEntities) {
	const world::SceneDescription scene = adaptTown();
	for (const world::ElementPlacement& element : scene.elements) {
		EXPECT_TRUE(element.depthSorted);
	}
}

TEST(MapSceneAdapterTest, AllElementsShareTheMapScale) {
	const world::SceneDescription scene = adaptTown();
	for (const world::ElementPlacement& element : scene.elements) {
		EXPECT_EQ(element.draw.mode, world::DrawMode::Single);
		EXPECT_EQ(element.draw.scaleX, kScale);
		EXPECT_EQ(element.draw.scaleY, kScale);
	}
}

// El contrato de coherencia: lo que se dibuja (y se clickea) de cada
// objeto es EXACTAMENTE el rectángulo de celdas que ocupa en el mapa.
TEST(MapSceneAdapterTest, DrawnBoundsEqualTheCellsTheObjectOccupies) {
	const world::SceneDescription scene = adaptTown();
	const world::ScenePixelMapper mapper(scene.cellPixelSize);
	for (const world::ElementPlacement& element : scene.elements) {
		const world::PixelRect area =
			mapper.toPixels(element.at, element.span);
		const world::ElementLayout layout(
			scene.tilesets.at(element.tileset), element, area);
		const world::PixelRect bounds = layout.bounds();
		EXPECT_EQ(bounds.x, area.x);
		EXPECT_EQ(bounds.y, area.y);
		EXPECT_EQ(bounds.width, area.width);
		EXPECT_EQ(bounds.height, area.height);
	}
}

TEST(MapSceneAdapterTest, ADifferentScaleKeepsTheCellsCoherent) {
	const world::SceneDescription scene = world::MapSceneAdapter(overlayWith(3)).adapt(
		editor_world_fixtures::makeWorld(), "town");
	EXPECT_EQ(scene.cellPixelSize, 48);
	const world::ScenePixelMapper mapper(scene.cellPixelSize);
	const world::ElementPlacement& market = scene.elements[1];
	const world::PixelRect area = mapper.toPixels(market.at, market.span);
	const world::ElementLayout layout(scene.tilesets.at(market.tileset),
					  market, area);
	EXPECT_EQ(layout.bounds().width, 2 * 48);
	EXPECT_EQ(layout.bounds().height, 2 * 48);
}

TEST(MapSceneAdapterTest, GroundBecomesALayerWithOneCellPerMapCell) {
	const world::SceneDescription scene = adaptTown();
	// "town" tiene piso y un camino: dos capas, el piso debajo.
	ASSERT_EQ(scene.layers.size(), 2u);
	const world::TileLayer& ground = scene.layers[0];
	EXPECT_EQ(ground.size.columns, 6);
	EXPECT_EQ(ground.size.rows, 5);
	ASSERT_EQ(ground.cells.size(), 30u);
	// La celda (0,0) está vacía; el resto usa scene_a:0,0.
	EXPECT_EQ(ground.cells[0].tilesetIndex, -1);
	ASSERT_EQ(ground.tilesets.size(), 1u);
	EXPECT_EQ(ground.tilesets[0], "scene_a");
	EXPECT_EQ(ground.cells[1].tilesetIndex, 0);
	EXPECT_EQ(ground.cells[1].sheetCell.column, 0);
	EXPECT_EQ(ground.cells[1].sheetCell.row, 0);
}

TEST(MapSceneAdapterTest, PathsLayerKeepsOnlyItsOwnTilesAtTheirCells) {
	const world::SceneDescription scene = adaptTown();
	const world::TileLayer& paths = scene.layers[1];
	int filled = 0;
	for (const world::LayerCell& cell : paths.cells) {
		filled += cell.tilesetIndex >= 0 ? 1 : 0;
	}
	EXPECT_EQ(filled, 1);
	// El camino está en (1,1): índice fila*ancho + columna.
	const world::LayerCell& cell = paths.cells[1 * 6 + 1];
	EXPECT_EQ(cell.tilesetIndex, 0);
	EXPECT_EQ(cell.sheetCell.column, 3);
	EXPECT_EQ(cell.sheetCell.row, 2);
}

TEST(MapSceneAdapterTest, EmptyPathsLayerIsOmitted) {
	const world::SceneDescription scene = world::MapSceneAdapter(overlayWith(kScale)).adapt(
		editor_world_fixtures::makeWorld(), "market");
	EXPECT_EQ(scene.layers.size(), 1u);
}

TEST(MapSceneAdapterTest, LayerWithTheWrongSizeIsInvalidWithItsPath) {
	worldmodel::World world = editor_world_fixtures::makeWorld();
	world.maps["town"].ground.pop_back();
	try {
		world::MapSceneAdapter(overlayWith(kScale)).adapt(world, "town");
		FAIL() << "se esperaba ConfigError";
	} catch (const world::ConfigError& error) {
		EXPECT_EQ(error.kind(), world::ConfigErrorKind::InvalidValue);
		EXPECT_EQ(error.path(), "maps.town.ground");
	}
}

TEST(MapSceneAdapterTest, LayerTilesetThatDoesNotFitTheCellIsInvalid) {
	worldmodel::World world = editor_world_fixtures::makeWorld();
	world.tilesets["scene_a"].cellWidth = 24;
	world.tilesets["scene_a"].cellHeight = 24;
	try {
		world::MapSceneAdapter(overlayWith(kScale)).adapt(world, "town");
		FAIL() << "se esperaba ConfigError";
	} catch (const world::ConfigError& error) {
		EXPECT_EQ(error.kind(), world::ConfigErrorKind::InvalidValue);
		EXPECT_EQ(error.path(), "tilesets.scene_a.cell");
	}
}

TEST(MapSceneAdapterTest, UnknownMapIsMissingStructureWithItsPath) {
	try {
		world::MapSceneAdapter(overlayWith(kScale)).adapt(
			editor_world_fixtures::makeWorld(), "nowhere");
		FAIL() << "se esperaba ConfigError";
	} catch (const world::ConfigError& error) {
		EXPECT_EQ(error.kind(), world::ConfigErrorKind::MissingStructure);
		EXPECT_EQ(error.path(), "maps.nowhere");
	}
}

TEST(MapSceneAdapterTest, TilesetCellThatDoesNotFitTheCellIsInvalid) {
	worldmodel::World world = editor_world_fixtures::makeWorld();
	world.tilesets["prefab_b"].cellWidth = 24;
	world.tilesets["prefab_b"].cellHeight = 24;
	try {
		world::MapSceneAdapter(overlayWith(kScale)).adapt(world, "town");
		FAIL() << "se esperaba ConfigError";
	} catch (const world::ConfigError& error) {
		EXPECT_EQ(error.kind(), world::ConfigErrorKind::InvalidValue);
		EXPECT_EQ(error.path(), "tilesets.prefab_b.cell");
	}
}

TEST(MapSceneAdapterTest, ObjectWithUnknownTilesetIsInvalidWithItsPath) {
	worldmodel::World world = editor_world_fixtures::makeWorld();
	world.maps["town"].objects[1].tile.tileset = "gone";
	try {
		world::MapSceneAdapter(overlayWith(kScale)).adapt(world, "town");
		FAIL() << "se esperaba ConfigError";
	} catch (const world::ConfigError& error) {
		EXPECT_EQ(error.kind(), world::ConfigErrorKind::InvalidValue);
		EXPECT_EQ(error.path(), "maps.town.objects[1].tile");
	}
}

// --- Overlay: lo que el cliente agrega al mundo del editor ------------

const char* const kOverlayToml = R"(
[world]
scale = 2

[tilesets.decoration]
image = "Decorations/Towninterior.png"
cell  = [16, 16]

[decorations."prefab_b:0,0"]
night_tile = "prefab_b:7,3"

[[maps.market.elements]]
tileset = "decoration"
region  = [17, 11, 1, 2]
draw    = { mode = "single" }
at      = [3, 1]
span    = [1, 2]
)";

TEST(MapSceneAdapterOverlayTest, NightVariantReplacesTheDecorationAtNight) {
	const world::WorldOverlay overlay =
		world::OverlayLoader().loadFromString(kOverlayToml);
	const world::SceneDescription scene =
		world::MapSceneAdapter(overlay).adapt(
			editor_world_fixtures::makeWorld(), "town");
	// El árbol (1x2) pasa a dos elementos consecutivos: día y noche.
	ASSERT_EQ(scene.elements.size(), 4u);
	const world::ElementPlacement& day = scene.elements[0];
	const world::ElementPlacement& night = scene.elements[1];
	ASSERT_TRUE(day.when.has_value());
	EXPECT_EQ(*day.when, world::TimeOfDay::Day);
	ASSERT_TRUE(night.when.has_value());
	EXPECT_EQ(*night.when, world::TimeOfDay::Night);
	// Misma posición y tamaño; la pieza arranca en la celda de noche.
	EXPECT_EQ(night.at.column, day.at.column);
	EXPECT_EQ(night.at.row, day.at.row);
	EXPECT_EQ(night.span.columns, day.span.columns);
	EXPECT_EQ(night.span.rows, day.span.rows);
	ASSERT_EQ(night.pieceCells.size(), 2u);
	EXPECT_EQ(night.pieceCells[0].column, 7);
	EXPECT_EQ(night.pieceCells[0].row, 3);
	EXPECT_EQ(night.pieceCells[1].row, 4);
}

TEST(MapSceneAdapterOverlayTest, DecorationWithoutOverlayEntryIsDrawnAlways) {
	const world::WorldOverlay overlay =
		world::OverlayLoader().loadFromString(kOverlayToml);
	const world::SceneDescription scene =
		world::MapSceneAdapter(overlay).adapt(
			editor_world_fixtures::makeWorld(), "town");
	// El mercado y el centro no tienen entrada: sin `when`.
	EXPECT_FALSE(scene.elements[2].when.has_value());
	EXPECT_FALSE(scene.elements[3].when.has_value());
}

TEST(MapSceneAdapterOverlayTest, InitialElementsAreAppendedAfterTheObjects) {
	const world::WorldOverlay overlay =
		world::OverlayLoader().loadFromString(kOverlayToml);
	const world::SceneDescription scene =
		world::MapSceneAdapter(overlay).adapt(
			editor_world_fixtures::makeWorld(), "market");
	ASSERT_EQ(scene.elements.size(), 1u);
	// Las decoraciones iniciales del cliente van debajo de todo: no se
	// ordenan con las entidades (como en scenes.toml).
	EXPECT_FALSE(scene.elements[0].depthSorted);
	EXPECT_EQ(scene.elements[0].tileset, "decoration");
	EXPECT_EQ(scene.elements[0].at.column, 3);
	// Su hoja es propia del cliente y queda disponible en la escena.
	ASSERT_EQ(scene.tilesets.count("decoration"), 1u);
	EXPECT_EQ(scene.tilesets.at("decoration").image,
		  "Decorations/Towninterior.png");
}

TEST(MapSceneAdapterOverlayTest, OverlayScaleDrivesTheCellSize) {
	const world::WorldOverlay overlay = world::OverlayLoader().loadFromString(
		"[world]\nscale = 3\n");
	const world::SceneDescription scene =
		world::MapSceneAdapter(overlay).adapt(
			editor_world_fixtures::makeWorld(), "town");
	EXPECT_EQ(scene.cellPixelSize, 48);
}

TEST(MapSceneAdapterOverlayTest, NightTilesetThatDoesNotExistIsInvalid) {
	const world::WorldOverlay overlay = world::OverlayLoader().loadFromString(R"(
[decorations."prefab_b:0,0"]
night_tile = "ghost:1,1"
)");
	try {
		world::MapSceneAdapter(overlay).adapt(
			editor_world_fixtures::makeWorld(), "town");
		FAIL() << "se esperaba ConfigError";
	} catch (const world::ConfigError& error) {
		EXPECT_EQ(error.kind(), world::ConfigErrorKind::InvalidValue);
		EXPECT_EQ(error.path(),
			  "decorations.prefab_b:0,0.night_tile");
	}
}

TEST(MapSceneAdapterOverlayTest, OverlayTilesetClashingWithTheWorldIsInvalid) {
	const world::WorldOverlay overlay = world::OverlayLoader().loadFromString(R"(
[tilesets.prefab_b]
image = "Other/other.png"
cell  = [16, 16]
)");
	try {
		world::MapSceneAdapter(overlay).adapt(
			editor_world_fixtures::makeWorld(), "town");
		FAIL() << "se esperaba ConfigError";
	} catch (const world::ConfigError& error) {
		EXPECT_EQ(error.kind(), world::ConfigErrorKind::InvalidValue);
		EXPECT_EQ(error.path(), "tilesets.prefab_b");
	}
}

TEST(OverlayLoaderTest, DefaultsWhenTheFileIsEmpty) {
	const world::WorldOverlay overlay =
		world::OverlayLoader().loadFromString("");
	EXPECT_EQ(overlay.scale, world::kDefaultWorldScale);
	EXPECT_TRUE(overlay.decorations.empty());
	EXPECT_TRUE(overlay.mapElements.empty());
}

TEST(OverlayLoaderTest, NonPositiveScaleIsInvalid) {
	try {
		world::OverlayLoader().loadFromString("[world]\nscale = 0\n");
		FAIL() << "se esperaba ConfigError";
	} catch (const world::ConfigError& error) {
		EXPECT_EQ(error.kind(), world::ConfigErrorKind::InvalidValue);
		EXPECT_EQ(error.path(), "world.scale");
	}
}

TEST(OverlayLoaderTest, BadNightTileTextIsInvalid) {
	try {
		world::OverlayLoader().loadFromString(R"(
[decorations."a:0,0"]
night_tile = "sin-formato"
)");
		FAIL() << "se esperaba ConfigError";
	} catch (const world::ConfigError& error) {
		EXPECT_EQ(error.kind(), world::ConfigErrorKind::InvalidValue);
	}
}

TEST(OverlayLoaderTest, ElementUsingAnUnknownTilesetIsInvalid) {
	try {
		world::OverlayLoader().loadFromString(R"(
[[maps.cabin.elements]]
tileset = "missing"
region  = [0, 0, 1, 1]
draw    = { mode = "single" }
at      = [0, 0]
span    = [1, 1]
)");
		FAIL() << "se esperaba ConfigError";
	} catch (const world::ConfigError& error) {
		EXPECT_EQ(error.kind(), world::ConfigErrorKind::InvalidValue);
	}
}

TEST(OverlayAuditTest, ReportsEntriesThatMatchNothing) {
	const world::WorldOverlay overlay = world::OverlayLoader().loadFromString(R"(
[tilesets.decoration]
image = "x.png"
cell  = [16, 16]

[decorations."prefab_b:0,0"]
night_tile = "prefab_b:1,1"

[decorations."prefab_b:9,9"]
night_tile = "prefab_b:1,1"

[[maps.nowhere.elements]]
tileset = "decoration"
region  = [0, 0, 1, 1]
draw    = { mode = "single" }
at      = [0, 0]
span    = [1, 1]
)");
	const std::vector<world::ConfigError> findings =
		world::OverlayAudit().unmatched(
			editor_world_fixtures::makeWorld(), overlay);
	ASSERT_EQ(findings.size(), 2u);
	EXPECT_EQ(findings[0].kind(), world::ConfigErrorKind::UnmatchedEntry);
	EXPECT_EQ(findings[0].path(), "decorations.prefab_b:9,9");
	EXPECT_EQ(findings[1].kind(), world::ConfigErrorKind::UnmatchedEntry);
	EXPECT_EQ(findings[1].path(), "maps.nowhere");
}

TEST(OverlayAuditTest, MatchingOverlayHasNoFindings) {
	const world::WorldOverlay overlay =
		world::OverlayLoader().loadFromString(kOverlayToml);
	EXPECT_TRUE(world::OverlayAudit()
			    .unmatched(editor_world_fixtures::makeWorld(), overlay)
			    .empty());
}

// --- Anchors: markers, portales, NPC y la cama -------------------------

TEST(MapSceneAdapterAnchorsTest, MarkersAndPortalsBecomeSceneData) {
	const world::SceneDescription market = world::MapSceneAdapter(
		overlayWith(kScale)).adapt(editor_world_fixtures::makeWorld(),
					   "market");
	ASSERT_EQ(market.markers.size(), 1u);
	EXPECT_EQ(market.markers[0].kind, "merchant_pierre");
	EXPECT_EQ(market.markers[0].cell.column, 2);
	EXPECT_EQ(market.markers[0].cell.row, 2);

	const world::SceneDescription town = adaptTown();
	ASSERT_EQ(town.portals.size(), 1u);
	EXPECT_EQ(town.portals[0].kind, "farm_entrance");
	EXPECT_EQ(town.portals[0].targetMap, "town");
	EXPECT_EQ(town.portals[0].cell.row, 4);
}

TEST(MapSceneAdapterAnchorsTest, MarkerBoundToAnNpcGivesItsDefaultCell) {
	const world::WorldOverlay overlay = world::OverlayLoader().loadFromString(R"(
[markers.merchant_pierre]
npc = "pierre"
)");
	const world::SceneDescription market =
		world::MapSceneAdapter(overlay).adapt(
			editor_world_fixtures::makeWorld(), "market");
	ASSERT_EQ(market.npcCells.count("pierre"), 1u);
	EXPECT_EQ(market.npcCells.at("pierre").column, 2);
	EXPECT_EQ(market.npcCells.at("pierre").row, 2);
}

TEST(MapSceneAdapterAnchorsTest, MarkersWithoutAnNpcBindingLeaveNoNpcCells) {
	const world::SceneDescription market = world::MapSceneAdapter(
		overlayWith(kScale)).adapt(editor_world_fixtures::makeWorld(),
					   "market");
	EXPECT_TRUE(market.npcCells.empty());
}

// El tile del "mueble" de la cabaña (cama 2x2 en (0,0)) lo agrega el
// test: el fixture solo trae el marker `bed` en (1,1).
worldmodel::World worldWithBed() {
	worldmodel::World world = editor_world_fixtures::makeWorld();
	world.maps["cabin"].objects.push_back(editor_world_fixtures::object(
		worldmodel::ObjectRole::Decoration,
		editor_world_fixtures::tile("prefab_b", 6, 0), 0, 0, 2, 4));
	return world;
}

const char* const kBedOverlay = R"(
[decorations."prefab_b:6,0"]
enter = { rows = [2, 3], facing = "right" }
)";

TEST(MapSceneAdapterAnchorsTest, BedMarkerGivesTheCellWhereThePlayerLies) {
	const world::WorldOverlay overlay =
		world::OverlayLoader().loadFromString(kBedOverlay);
	const world::SceneDescription cabin =
		world::MapSceneAdapter(overlay).adapt(worldWithBed(), "cabin");
	ASSERT_EQ(cabin.elements.size(), 1u);
	ASSERT_TRUE(cabin.elements[0].enter.has_value());
	const world::ElementEnter& enter = *cabin.elements[0].enter;
	EXPECT_EQ(enter.lie.column, 1);
	EXPECT_EQ(enter.lie.row, 1);
	EXPECT_EQ(enter.firstFrontRow, 2);
	EXPECT_EQ(enter.lastFrontRow, 3);
	EXPECT_EQ(enter.facing, world::Direction::Right);
}

TEST(MapSceneAdapterAnchorsTest, EnterWithoutItsMarkerIsMissingStructure) {
	worldmodel::World world = worldWithBed();
	world.maps["cabin"].markers.clear();
	const world::WorldOverlay overlay =
		world::OverlayLoader().loadFromString(kBedOverlay);
	try {
		world::MapSceneAdapter(overlay).adapt(world, "cabin");
		FAIL() << "se esperaba ConfigError";
	} catch (const world::ConfigError& error) {
		EXPECT_EQ(error.kind(), world::ConfigErrorKind::MissingStructure);
		EXPECT_EQ(error.path(), "maps.cabin.markers");
	}
}

TEST(MapSceneAdapterAnchorsTest, MarkerOutsideTheDecorationIsInvalid) {
	worldmodel::World world = worldWithBed();
	world.maps["cabin"].markers[0].cell = {3, 3};
	const world::WorldOverlay overlay =
		world::OverlayLoader().loadFromString(kBedOverlay);
	try {
		world::MapSceneAdapter(overlay).adapt(world, "cabin");
		FAIL() << "se esperaba ConfigError";
	} catch (const world::ConfigError& error) {
		EXPECT_EQ(error.kind(), world::ConfigErrorKind::InvalidValue);
		EXPECT_EQ(error.path(), "maps.cabin.markers");
	}
}

TEST(MapSceneAdapterAnchorsTest, FrontRowsBeyondThePieceAreInvalid) {
	const world::WorldOverlay overlay = world::OverlayLoader().loadFromString(R"(
[decorations."prefab_b:6,0"]
enter = { rows = [2, 9], facing = "right" }
)");
	try {
		world::MapSceneAdapter(overlay).adapt(worldWithBed(), "cabin");
		FAIL() << "se esperaba ConfigError";
	} catch (const world::ConfigError& error) {
		EXPECT_EQ(error.kind(), world::ConfigErrorKind::InvalidValue);
		EXPECT_EQ(error.path(), "decorations.prefab_b:6,0.enter.rows");
	}
}

TEST(MapSceneAdapterAnchorsTest, EnterAndNightVariantTogetherAreInvalid) {
	const world::WorldOverlay overlay = world::OverlayLoader().loadFromString(R"(
[decorations."prefab_b:6,0"]
night_tile = "prefab_b:0,0"
enter = { rows = [2, 3], facing = "right" }
)");
	try {
		world::MapSceneAdapter(overlay).adapt(worldWithBed(), "cabin");
		FAIL() << "se esperaba ConfigError";
	} catch (const world::ConfigError& error) {
		EXPECT_EQ(error.kind(), world::ConfigErrorKind::InvalidValue);
		EXPECT_EQ(error.path(), "decorations.prefab_b:6,0");
	}
}

TEST(OverlayAuditTest, UnknownMarkerKindIsReported) {
	const world::WorldOverlay overlay = world::OverlayLoader().loadFromString(R"(
[markers.merchant_nadie]
npc = "nadie"
)");
	const std::vector<world::ConfigError> findings =
		world::OverlayAudit().unmatched(
			editor_world_fixtures::makeWorld(), overlay);
	ASSERT_EQ(findings.size(), 1u);
	EXPECT_EQ(findings[0].path(), "markers.merchant_nadie");
}

TEST(EditorWorldSourceTest, KnowsTheMapsOfTheWorld) {
	const world::EditorWorldSource source(
		editor_world_fixtures::makeWorld(), overlayWith(kScale));
	EXPECT_TRUE(source.has("town"));
	EXPECT_FALSE(source.has("bedroom"));
	EXPECT_EQ(source.ids().size(), 3u);
	EXPECT_EQ(source.load("market").grid.columns, 5);
}

class SceneSourcesTest : public ::testing::Test {
protected:
	void SetUp() override {
		worldPath_ = std::string(::testing::TempDir()) +
			     "scene_sources_world.toml";
		std::ofstream out(worldPath_);
		out << editor_world_fixtures::makeToml();
	}

	void TearDown() override { std::remove(worldPath_.c_str()); }

	std::string worldPath_;
};

#ifdef TALLER_SCENES_TOML
TEST_F(SceneSourcesTest, WithoutWorldFileOnlyTheRoomsAreUsed) {
	const world::SceneSources sources(TALLER_SCENES_TOML,
					  "/no/existe/world.toml",
					  "/no/existe/overlay.toml");
	EXPECT_EQ(sources.byPriority().size(), 1u);
}

TEST_F(SceneSourcesTest, EditorMapsWinOverRoomsWithTheSameId) {
	const world::SceneSources sources(TALLER_SCENES_TOML, worldPath_,
					  "/no/existe/overlay.toml");
	const std::vector<const world::SceneSource*> byPriority =
		sources.byPriority();
	ASSERT_EQ(byPriority.size(), 2u);
	// "market" existe en las dos: gana el mapa del editor (5x4).
	EXPECT_EQ(byPriority[0]->load("market").grid.columns, 5);
	// "bedroom" solo está en scenes.toml.
	EXPECT_FALSE(byPriority[0]->has("bedroom"));
	EXPECT_TRUE(byPriority[1]->has("bedroom"));
}

TEST_F(SceneSourcesTest, OverlayFileIsAppliedAndUnmatchedEntriesAreReported) {
	const std::string overlayPath = std::string(::testing::TempDir()) +
					"scene_sources_overlay.toml";
	{
		std::ofstream out(overlayPath);
		out << "[world]\nscale = 3\n"
		       "[decorations.\"prefab_b:9,9\"]\n"
		       "night_tile = \"prefab_b:1,1\"\n";
	}
	const world::SceneSources sources(TALLER_SCENES_TOML, worldPath_,
					  overlayPath);
	std::remove(overlayPath.c_str());
	// La escala del overlay llega a las escenas del editor.
	EXPECT_EQ(sources.byPriority()[0]->load("town").cellPixelSize, 48);
	const std::vector<world::ConfigError> findings = sources.findings();
	ASSERT_EQ(findings.size(), 1u);
	EXPECT_EQ(findings[0].path(), "decorations.prefab_b:9,9");
}
#endif

}  // namespace
