#include <string>

#include "gtest/gtest.h"
#include "scene_fixtures.h"
#include "world/SceneConfigLoader.h"

namespace {

using scene_fixtures::kBedroomToml;
using scene_fixtures::loadBedroom;
using scene_fixtures::loadString;


TEST(SceneConfigLoaderTest, LoadsTileSet) {
	const world::SceneDescription scene = loadBedroom();
	ASSERT_EQ(scene.tilesets.count("farm"), 1u);
	const world::TileSetDef& farm = scene.tilesets.at("farm");
	EXPECT_EQ(farm.image, "Farm/walls&floors.png");
	EXPECT_EQ(farm.cellWidth, 16);
	EXPECT_EQ(farm.cellHeight, 16);
}

TEST(SceneConfigLoaderTest, LoadsRoomFloorColorAndGrid) {
	const world::SceneDescription scene = loadBedroom();
	EXPECT_EQ(scene.floorColor.r, 0);
	EXPECT_EQ(scene.floorColor.g, 0);
	EXPECT_EQ(scene.floorColor.b, 0);
	EXPECT_EQ(scene.floorColor.a, 255);
	EXPECT_EQ(scene.grid.columns, 15);
	EXPECT_EQ(scene.grid.rows, 10);
}

TEST(SceneConfigLoaderTest, LoadsElementsInOrder) {
	const world::SceneDescription scene = loadBedroom();
	ASSERT_EQ(scene.elements.size(), 2u);

	const world::ElementPlacement& floor = scene.elements[0];
	EXPECT_EQ(floor.tileset, "farm");
	EXPECT_EQ(floor.region.origin.column, 0);
	EXPECT_EQ(floor.region.origin.row, 27);
	EXPECT_EQ(floor.region.size.columns, 2);
	EXPECT_EQ(floor.region.size.rows, 2);
	EXPECT_EQ(floor.at.column, 0);
	EXPECT_EQ(floor.at.row, 2);
	EXPECT_EQ(floor.span.columns, 15);
	EXPECT_EQ(floor.span.rows, 8);
	EXPECT_EQ(floor.draw.mode, world::DrawMode::Tile);

	const world::ElementPlacement& wall = scene.elements[1];
	EXPECT_EQ(wall.draw.mode, world::DrawMode::HorizontalStrip);
}

TEST(SceneConfigLoaderTest, ElementWithoutScaleInheritsRoomDefault) {
	const world::SceneDescription scene = loadBedroom();
	const world::ElementPlacement& floor = scene.elements[0];
	EXPECT_EQ(floor.draw.scaleX, 2);
	EXPECT_EQ(floor.draw.scaleY, 2);
}

TEST(SceneConfigLoaderTest, ElementScaleOverridesRoomDefault) {
	const world::SceneDescription scene = loadBedroom();
	const world::ElementPlacement& wall = scene.elements[1];
	EXPECT_EQ(wall.draw.scaleX, 2);
	EXPECT_EQ(wall.draw.scaleY, 3);
}

TEST(SceneConfigLoaderTest, UniformScaleAppliesToBothAxes) {
	const std::string toml = R"(
[tilesets.farm]
image = "x.png"
cell  = [16, 16]
[rooms.bedroom]
floor_color = [0,0,0,255]
grid = [4,4]
scale = 2
  [[rooms.bedroom.elements]]
  tileset = "farm"
  region  = [0,0,1,1]
  draw    = { mode = "single", scale = 4 }
  at      = [0,0]
  span    = [1,1]
)";
	const world::SceneDescription scene = loadString(toml, "bedroom");
	ASSERT_EQ(scene.elements.size(), 1u);
	EXPECT_EQ(scene.elements[0].draw.scaleX, 4);
	EXPECT_EQ(scene.elements[0].draw.scaleY, 4);
}

TEST(SceneConfigLoaderTest, MissingRoomThrows) {
	EXPECT_THROW(loadString(kBedroomToml, "kitchen"),
		     world::ConfigError);
}

TEST(SceneConfigLoaderTest, RoomWithoutSoundsIsSilent) {
	// La key `sounds` es OPCIONAL: una sala sin ella no declara
	// cues y la presentación no reproduce nada ahí.
	const world::SceneDescription scene = loadBedroom();
	EXPECT_TRUE(scene.soundCues.empty());
}

TEST(SceneConfigLoaderTest, LoadsRoomSoundCues) {
	const std::string toml = R"(
[tilesets.farm]
image = "x.png"
cell  = [16, 16]
[rooms.barn]
floor_color = [0,0,0,255]
grid = [4,4]
scale = 2
sounds = ["chicken_cluck", "rooster"]
  [[rooms.barn.elements]]
  tileset = "farm"
  region  = [0,0,1,1]
  draw    = { mode = "single" }
  at      = [0,0]
  span    = [1,1]
)";
	const world::SceneDescription scene = loadString(toml, "barn");
	ASSERT_EQ(scene.soundCues.size(), 2u);
	EXPECT_EQ(scene.soundCues[0], "chicken_cluck");
	EXPECT_EQ(scene.soundCues[1], "rooster");
}

TEST(SceneConfigLoaderTest, InvalidTomlThrows) {
	EXPECT_THROW(loadString("not = = toml", "bedroom"),
		     world::ConfigError);
}

#ifdef TALLER_SCENES_TOML
TEST(SceneConfigLoaderTest, LoadsRealBedroomFile) {
	world::SceneConfigLoader loader;
	const world::SceneDescription scene =
		loader.loadFromFile(TALLER_SCENES_TOML, "bedroom");
	ASSERT_EQ(scene.tilesets.count("farm"), 1u);
	ASSERT_EQ(scene.elements.size(), 2u);
	EXPECT_EQ(scene.elements[0].draw.mode, world::DrawMode::Tile);
	EXPECT_EQ(scene.elements[1].draw.mode,
		  world::DrawMode::HorizontalStrip);
	EXPECT_EQ(scene.elements[1].draw.scaleY, 3);
	// La sala inicial declara sus cues audibles (cacareos: las
	// gallinas están acá hoy; el granero los declarará mañana).
	ASSERT_EQ(scene.soundCues.size(), 1u);
	EXPECT_EQ(scene.soundCues[0], "chicken_cluck");
}
#endif


TEST(SceneConfigLoaderTest, ClassifiesUnknownDrawModeAsInvalidValue) {
	const std::string toml = R"(
[tilesets.farm]
image = "x.png"
cell  = [16, 16]
[rooms.bedroom]
floor_color = [0,0,0,255]
grid = [4,4]
scale = 2
  [[rooms.bedroom.elements]]
  tileset = "farm"
  region  = [0,0,1,1]
  draw    = { mode = "diagonal" }
  at      = [0,0]
  span    = [1,1]
)";
	try {
		loadString(toml, "bedroom");
		FAIL() << "se esperaba ConfigError";
	} catch (const world::ConfigError& error) {
		EXPECT_EQ(error.kind(),
			  world::ConfigErrorKind::InvalidValue);
	}
}

TEST(SceneConfigLoaderTest, ClassifiesMissingRoomAsMissingStructure) {
	try {
		loadString(kBedroomToml, "kitchen");
		FAIL() << "se esperaba ConfigError";
	} catch (const world::ConfigError& error) {
		EXPECT_EQ(error.kind(),
			  world::ConfigErrorKind::MissingStructure);
	}
}

TEST(SceneConfigLoaderTest, ClassifiesUnknownTileSetAsInvalidValue) {
	const std::string toml = R"(
[tilesets.farm]
image = "x.png"
cell  = [16, 16]
[rooms.bedroom]
floor_color = [0,0,0,255]
grid = [4,4]
scale = 2
  [[rooms.bedroom.elements]]
  tileset = "missing"
  region  = [0,0,1,1]
  draw    = { mode = "single" }
  at      = [0,0]
  span    = [1,1]
)";
	try {
		loadString(toml, "bedroom");
		FAIL() << "se esperaba ConfigError";
	} catch (const world::ConfigError& error) {
		EXPECT_EQ(error.kind(),
			  world::ConfigErrorKind::InvalidValue);
	}
}

TEST(SceneConfigLoaderTest, ClassifiesWrongTypeArray) {
	const std::string toml = R"(
[tilesets.farm]
image = "x.png"
cell  = [16, 16]
[rooms.bedroom]
floor_color = [0,0,0,255]
grid = "no-es-una-lista"
scale = 2
  [[rooms.bedroom.elements]]
  tileset = "farm"
  region  = [0,0,1,1]
  draw    = { mode = "single" }
  at      = [0,0]
  span    = [1,1]
)";
	try {
		loadString(toml, "bedroom");
		FAIL() << "se esperaba ConfigError";
	} catch (const world::ConfigError& error) {
		EXPECT_EQ(error.kind(),
			  world::ConfigErrorKind::WrongType);
	}
}

}
