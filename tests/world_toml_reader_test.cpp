#include <cassert>
#include <optional>
#include <string>

#include "gtest/gtest.h"
#include "world_fixtures.h"
#include "world_model/FormatError.h"
#include "world_model/WorldSerializer.h"

namespace {

using worldmodel::FormatError;
using worldmodel::FormatErrorKind;
using worldmodel::MapKind;
using worldmodel::MarkerKind;
using worldmodel::ObjectRole;
using worldmodel::PortalKind;
using worldmodel::World;
using worldmodel::WorldSerializer;

// Mundo chico con todas las secciones para los tests de error; cada test
// reemplaza exactamente un fragmento.
const std::string kSmallWorld = R"(format_version = 1

[world]
slug = "x"
name = "X"

[tilesets.outdoors]
image = "a.png"
cell = [16, 16]

[maps.farm]
kind = "farm"
width = 1
height = 1
tile_size = 16
base_collision = [false]
collision = [false]
ground = ["outdoors:0,0"]

[[maps.farm.objects]]
role = "decoration"
tile = "outdoors:0,0"
cell = [0, 0]

[[maps.farm.markers]]
kind = "player_spawn"
cell = [0, 0]

[[maps.farm.portals]]
kind = "link"
cell = [0, 0]
target_map = "farm"
target_cell = [0, 0]
)";

std::string replaced(std::string text, const std::string& from,
		     const std::string& to) {
	const std::size_t pos = text.find(from);
	assert(pos != std::string::npos);
	text.replace(pos, from.size(), to);
	return text;
}

void expectError(const std::string& toml, FormatErrorKind kind,
		 const std::string& path) {
	try {
		(void)WorldSerializer().loadFromString(toml);
		FAIL() << "expected FormatError at " << path;
	} catch (const FormatError& error) {
		EXPECT_EQ(error.kind(), kind) << error.what();
		EXPECT_EQ(error.path(), path);
	}
}

TEST(WorldTomlReaderTest, ReadsWorldHeaderAndTilesets) {
	const World world = worldfixtures::loadMinimalWorld();
	EXPECT_EQ(world.formatVersion, 1);
	EXPECT_EQ(world.slug, "spring");
	EXPECT_EQ(world.name, "Primavera");
	ASSERT_EQ(world.tilesets.size(), 2u);
	const auto& outdoors = world.tilesets.at("outdoors");
	EXPECT_EQ(outdoors.image, "Farm/spring_outdoors.png");
	EXPECT_EQ(outdoors.cellWidth, 16);
	EXPECT_EQ(outdoors.cellHeight, 16);
	EXPECT_EQ(world.tilesets.at("interiors").image,
		  "Interiors/cabin.png");
}

TEST(WorldTomlReaderTest, ReadsMapsAndGrids) {
	const World world = worldfixtures::loadMinimalWorld();
	ASSERT_EQ(world.maps.size(), 4u);
	const auto& farm = world.maps.at("farm");
	EXPECT_EQ(farm.id, "farm");
	EXPECT_EQ(farm.kind, MapKind::Farm);
	EXPECT_EQ(farm.level, 0);
	EXPECT_EQ(farm.width, 3);
	EXPECT_EQ(farm.height, 2);
	EXPECT_EQ(farm.tileSize, 16);
	EXPECT_EQ(farm.baseCollision.size(), 6u);
	EXPECT_EQ(farm.collision.size(), 6u);
	ASSERT_EQ(farm.ground.size(), 6u);
	ASSERT_TRUE(farm.ground[0].has_value());
	EXPECT_EQ(farm.ground[0]->tileset, "outdoors");
	EXPECT_EQ(farm.ground[0]->column, 0);
	EXPECT_EQ(farm.ground[0]->row, 0);
	ASSERT_TRUE(farm.ground[1].has_value());
	EXPECT_EQ(farm.ground[1]->column, 1);
	ASSERT_EQ(farm.paths.size(), 6u);
	EXPECT_FALSE(farm.paths[0].has_value());
	ASSERT_TRUE(farm.paths[4].has_value());
	EXPECT_EQ(farm.paths[4]->column, 5);
}

TEST(WorldTomlReaderTest, ReadsObjectMarkerAndPortal) {
	const World world = worldfixtures::loadMinimalWorld();
	const auto& farm = world.maps.at("farm");

	ASSERT_EQ(farm.objects.size(), 1u);
	const auto& object = farm.objects[0];
	EXPECT_EQ(object.role, ObjectRole::Cabin);
	EXPECT_EQ(object.tile.tileset, "outdoors");
	EXPECT_EQ(object.tile.column, 10);
	EXPECT_EQ(object.cell.column, 0);
	EXPECT_EQ(object.cell.row, 0);
	EXPECT_EQ(object.width, 2);
	EXPECT_EQ(object.height, 1);
	EXPECT_TRUE(object.blocked);
	ASSERT_TRUE(object.door.has_value());
	EXPECT_EQ(object.door->column, 0);
	EXPECT_EQ(object.door->row, 1);

	ASSERT_EQ(farm.markers.size(), 1u);
	EXPECT_EQ(farm.markers[0].kind, MarkerKind::PlayerSpawn);
	EXPECT_EQ(farm.markers[0].cell.column, 2);
	EXPECT_EQ(farm.markers[0].cell.row, 1);

	ASSERT_EQ(farm.portals.size(), 1u);
	EXPECT_EQ(farm.portals[0].kind, PortalKind::TownEntrance);
	EXPECT_EQ(farm.portals[0].cell.column, 2);
	EXPECT_EQ(farm.portals[0].targetMap, "town");
	EXPECT_EQ(farm.portals[0].targetCell.column, 0);
	EXPECT_EQ(farm.portals[0].targetCell.row, 1);
}

TEST(WorldTomlReaderTest, ReadsExtraMapAndLink) {
	const World world = worldfixtures::loadMinimalWorld();
	const auto& extra = world.maps.at("secret_cave");
	EXPECT_EQ(extra.kind, MapKind::Dungeon);
	EXPECT_EQ(extra.level, 3);
	EXPECT_EQ(extra.width, 1);
	EXPECT_EQ(extra.height, 1);
	EXPECT_TRUE(extra.objects.empty());
	EXPECT_TRUE(extra.markers.empty());
	EXPECT_TRUE(extra.portals.empty());

	const auto& town = world.maps.at("town");
	ASSERT_EQ(town.portals.size(), 1u);
	EXPECT_EQ(town.portals[0].kind, PortalKind::Link);
	EXPECT_EQ(town.portals[0].targetMap, "secret_cave");
}

TEST(WorldTomlReaderTest, AppliesDefaultsForOmittedFields) {
	const World world = WorldSerializer().loadFromString(kSmallWorld);
	const auto& farm = world.maps.at("farm");
	ASSERT_EQ(farm.objects.size(), 1u);
	EXPECT_EQ(farm.objects[0].width, 1);
	EXPECT_EQ(farm.objects[0].height, 1);
	EXPECT_FALSE(farm.objects[0].blocked);
	EXPECT_FALSE(farm.objects[0].door.has_value());

	const World minimal = worldfixtures::loadMinimalWorld();
	const auto& town = minimal.maps.at("town");
	ASSERT_EQ(town.paths.size(), 6u);
	for (const auto& tile : town.paths) {
		EXPECT_FALSE(tile.has_value());
	}
	EXPECT_TRUE(town.objects.empty());
	EXPECT_TRUE(town.markers.empty());
	EXPECT_EQ(minimal.maps.at("dungeon_1").level, 1);
}

TEST(WorldTomlReaderTest, MissingFormatVersionIsMissingStructure) {
	const std::string toml = replaced(kSmallWorld, "format_version = 1\n", "");
	expectError(toml, FormatErrorKind::MissingStructure,
		    "format_version");
}

TEST(WorldTomlReaderTest, UnsupportedVersionIsInvalid) {
	expectError(replaced(kSmallWorld, "format_version = 1",
			     "format_version = 2"),
		    FormatErrorKind::InvalidValue, "format_version");
}

TEST(WorldTomlReaderTest, GroundSizeMismatchReportsGroundPath) {
	expectError(replaced(kSmallWorld, "ground = [\"outdoors:0,0\"]",
			     "ground = [\"outdoors:0,0\", \"outdoors:0,0\"]"),
		    FormatErrorKind::InvalidValue, "maps.farm.ground");
}

TEST(WorldTomlReaderTest, UnknownMapKindReportsKindPath) {
	expectError(replaced(kSmallWorld, "kind = \"farm\"", "kind = \"nope\""),
		    FormatErrorKind::InvalidValue, "maps.farm.kind");
}

TEST(WorldTomlReaderTest, UnknownTilesetInGroundReportsCellPath) {
	expectError(replaced(kSmallWorld, "ground = [\"outdoors:0,0\"]",
			     "ground = [\"nope:0,0\"]"),
		    FormatErrorKind::InvalidValue, "maps.farm.ground[0]");
}

TEST(WorldTomlReaderTest, InvalidTileEncodingsReportCellPath) {
	for (const char* encoding : {"outdoors:x,0", "outdoors:0",
				     "outdoors:0,0,0"}) {
		const std::string toml = replaced(
			kSmallWorld, "ground = [\"outdoors:0,0\"]",
			std::string("ground = [\"") + encoding + "\"]");
		expectError(toml, FormatErrorKind::InvalidValue,
			    "maps.farm.ground[0]");
	}
}

TEST(WorldTomlReaderTest, InvalidObjectTileReportsIndexedPath) {
	expectError(replaced(kSmallWorld, "tile = \"outdoors:0,0\"",
			     "tile = \"outdoors:x,0\""),
		    FormatErrorKind::InvalidValue,
		    "maps.farm.objects[0].tile");
}

TEST(WorldTomlReaderTest, UnknownObjectRoleReportsIndexedPath) {
	expectError(replaced(kSmallWorld, "role = \"decoration\"",
			     "role = \"nope\""),
		    FormatErrorKind::InvalidValue,
		    "maps.farm.objects[0].role");
}

TEST(WorldTomlReaderTest, UnknownMarkerKindReportsIndexedPath) {
	expectError(replaced(kSmallWorld, "kind = \"player_spawn\"",
			     "kind = \"nope\""),
		    FormatErrorKind::InvalidValue,
		    "maps.farm.markers[0].kind");
}

TEST(WorldTomlReaderTest, UnknownPortalKindReportsIndexedPath) {
	expectError(replaced(kSmallWorld, "kind = \"link\"", "kind = \"nope\""),
		    FormatErrorKind::InvalidValue,
		    "maps.farm.portals[0].kind");
}

TEST(WorldTomlReaderTest, EmptyPortalTargetMapIsInvalid) {
	expectError(replaced(kSmallWorld, "target_map = \"farm\"",
			     "target_map = \"\""),
		    FormatErrorKind::InvalidValue,
		    "maps.farm.portals[0].target_map");
}

TEST(WorldTomlReaderTest, InvalidTomlIsUnreadable) {
	try {
		(void)WorldSerializer().loadFromString("no = = toml");
		FAIL() << "expected FormatError";
	} catch (const FormatError& error) {
		EXPECT_EQ(error.kind(), FormatErrorKind::UnreadableInput);
	}
}

}
