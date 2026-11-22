#include <cstdio>
#include <fstream>
#include <string>

#include "editor_world_fixtures.h"
#include "gtest/gtest.h"
#include "world/ConfigError.h"
#include "world/EditorWorldSource.h"
#include "world/ElementLayout.h"
#include "world/MapSceneAdapter.h"
#include "world/SceneSources.h"
#include "world/ScenePixelMapper.h"

namespace {

constexpr int kScale = 2;

world::SceneDescription adaptTown() {
	return world::MapSceneAdapter(kScale).adapt(
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
	const world::SceneDescription scene = world::MapSceneAdapter(3).adapt(
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
	const world::SceneDescription scene = world::MapSceneAdapter(kScale).adapt(
		editor_world_fixtures::makeWorld(), "market");
	EXPECT_EQ(scene.layers.size(), 1u);
}

TEST(MapSceneAdapterTest, LayerWithTheWrongSizeIsInvalidWithItsPath) {
	worldmodel::World world = editor_world_fixtures::makeWorld();
	world.maps["town"].ground.pop_back();
	try {
		world::MapSceneAdapter(kScale).adapt(world, "town");
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
		world::MapSceneAdapter(kScale).adapt(world, "town");
		FAIL() << "se esperaba ConfigError";
	} catch (const world::ConfigError& error) {
		EXPECT_EQ(error.kind(), world::ConfigErrorKind::InvalidValue);
		EXPECT_EQ(error.path(), "tilesets.scene_a.cell");
	}
}

TEST(MapSceneAdapterTest, UnknownMapIsMissingStructureWithItsPath) {
	try {
		world::MapSceneAdapter(kScale).adapt(
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
		world::MapSceneAdapter(kScale).adapt(world, "town");
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
		world::MapSceneAdapter(kScale).adapt(world, "town");
		FAIL() << "se esperaba ConfigError";
	} catch (const world::ConfigError& error) {
		EXPECT_EQ(error.kind(), world::ConfigErrorKind::InvalidValue);
		EXPECT_EQ(error.path(), "maps.town.objects[1].tile");
	}
}

TEST(EditorWorldSourceTest, KnowsTheMapsOfTheWorld) {
	const world::EditorWorldSource source(
		editor_world_fixtures::makeWorld(), kScale);
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
					  "/no/existe/world.toml", kScale);
	EXPECT_EQ(sources.byPriority().size(), 1u);
}

TEST_F(SceneSourcesTest, EditorMapsWinOverRoomsWithTheSameId) {
	const world::SceneSources sources(TALLER_SCENES_TOML, worldPath_,
					  kScale);
	const std::vector<const world::SceneSource*> byPriority =
		sources.byPriority();
	ASSERT_EQ(byPriority.size(), 2u);
	// "market" existe en las dos: gana el mapa del editor (5x4).
	EXPECT_EQ(byPriority[0]->load("market").grid.columns, 5);
	// "bedroom" solo está en scenes.toml.
	EXPECT_FALSE(byPriority[0]->has("bedroom"));
	EXPECT_TRUE(byPriority[1]->has("bedroom"));
}
#endif

}  // namespace
