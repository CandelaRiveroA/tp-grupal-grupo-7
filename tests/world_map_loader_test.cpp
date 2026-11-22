#include <string>

#include "editor_world_fixtures.h"
#include "gtest/gtest.h"
#include "world/ConfigError.h"
#include "world/WorldMapLoader.h"

namespace {

TEST(WorldMapLoaderTest, LoadsTheMapsOfTheEditorWorld) {
	const worldmodel::World world =
		world::WorldMapLoader().loadFromString(
			editor_world_fixtures::makeToml());
	EXPECT_EQ(world.maps.size(), 3u);
	ASSERT_EQ(world.maps.count("town"), 1u);
	const worldmodel::Map& town = world.maps.at("town");
	EXPECT_EQ(town.width, 6);
	EXPECT_EQ(town.height, 5);
	EXPECT_EQ(town.tileSize, 16);
	EXPECT_EQ(town.objects.size(), 3u);
}

TEST(WorldMapLoaderTest, KeepsObjectsInTheFileOrder) {
	const worldmodel::World world =
		world::WorldMapLoader().loadFromString(
			editor_world_fixtures::makeToml());
	const worldmodel::Map& town = world.maps.at("town");
	EXPECT_EQ(town.objects[0].role, worldmodel::ObjectRole::Decoration);
	EXPECT_EQ(town.objects[1].role, worldmodel::ObjectRole::Market);
	EXPECT_EQ(town.objects[2].role,
		  worldmodel::ObjectRole::CommunityCenter);
}

TEST(WorldMapLoaderTest, UnknownTilesetIsAConfigErrorWithTheRawPath) {
	std::string toml = editor_world_fixtures::makeToml();
	const std::string::size_type at = toml.find("prefab_b:0,0");
	ASSERT_NE(at, std::string::npos);
	toml.replace(at, std::string("prefab_b").size(), "missing_set");
	try {
		world::WorldMapLoader().loadFromString(toml);
		FAIL() << "se esperaba ConfigError";
	} catch (const world::ConfigError& error) {
		EXPECT_EQ(error.kind(), world::ConfigErrorKind::InvalidValue);
		EXPECT_FALSE(error.path().empty());
	}
}

TEST(WorldMapLoaderTest, BrokenTomlIsUnreadableInput) {
	try {
		world::WorldMapLoader().loadFromString("not = = toml");
		FAIL() << "se esperaba ConfigError";
	} catch (const world::ConfigError& error) {
		EXPECT_EQ(error.kind(), world::ConfigErrorKind::UnreadableInput);
	}
}

TEST(WorldMapLoaderTest, MissingFileIsUnreadableInput) {
	try {
		world::WorldMapLoader().loadFromFile("/no/existe/world.toml");
		FAIL() << "se esperaba ConfigError";
	} catch (const world::ConfigError& error) {
		EXPECT_EQ(error.kind(), world::ConfigErrorKind::UnreadableInput);
	}
}

}  // namespace
