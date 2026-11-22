#include <string>

#include "gtest/gtest.h"
#include "world_fixtures.h"
#include "world_model/FormatError.h"
#include "world_model/WorldSerializer.h"

namespace {

using worldmodel::FormatError;
using worldmodel::FormatErrorKind;
using worldmodel::MapKind;
using worldmodel::PortalKind;
using worldmodel::World;
using worldmodel::WorldSerializer;

TEST(WorldSerializerTest, RoundTripPreservesWorld) {
	const WorldSerializer serializer;
	const World first = worldfixtures::loadMinimalWorld();
	const World second =
		serializer.loadFromString(serializer.saveToString(first));

	EXPECT_EQ(second.formatVersion, first.formatVersion);
	EXPECT_EQ(second.name, first.name);
	ASSERT_EQ(second.tilesets.size(), first.tilesets.size());
	ASSERT_EQ(second.maps.size(), first.maps.size());

	const auto& farm = second.maps.at("farm");
	const auto& original = first.maps.at("farm");
	EXPECT_EQ(farm.kind, MapKind::Farm);
	EXPECT_EQ(farm.width, original.width);
	EXPECT_EQ(farm.height, original.height);
	EXPECT_EQ(farm.tileSize, original.tileSize);
	EXPECT_EQ(farm.baseCollision, original.baseCollision);
	EXPECT_EQ(farm.collision, original.collision);
	EXPECT_EQ(farm.ground, original.ground);
	EXPECT_EQ(farm.paths, original.paths);

	ASSERT_EQ(farm.objects.size(), 1u);
	EXPECT_EQ(farm.objects[0].role, original.objects[0].role);
	EXPECT_EQ(farm.objects[0].tile, original.objects[0].tile);
	EXPECT_EQ(farm.objects[0].cell, original.objects[0].cell);
	EXPECT_EQ(farm.objects[0].width, original.objects[0].width);
	EXPECT_EQ(farm.objects[0].height, original.objects[0].height);
	EXPECT_EQ(farm.objects[0].blocked, original.objects[0].blocked);
	EXPECT_EQ(farm.objects[0].door, original.objects[0].door);

	ASSERT_EQ(farm.markers.size(), 1u);
	EXPECT_EQ(farm.markers[0].kind, original.markers[0].kind);
	EXPECT_EQ(farm.markers[0].cell, original.markers[0].cell);

	ASSERT_EQ(farm.portals.size(), 1u);
	EXPECT_EQ(farm.portals[0].kind, original.portals[0].kind);
	EXPECT_EQ(farm.portals[0].cell, original.portals[0].cell);
	EXPECT_EQ(farm.portals[0].targetMap, original.portals[0].targetMap);
	EXPECT_EQ(farm.portals[0].targetCell, original.portals[0].targetCell);
	EXPECT_EQ(farm.portals[0].width, original.portals[0].width);
	EXPECT_EQ(farm.portals[0].height, original.portals[0].height);

	World multi = first;
	multi.maps.at("farm").front.assign(multi.maps.at("farm").ground.size(),
					   std::nullopt);
	multi.maps.at("farm").front[0] = worldmodel::TileRef{"outdoors", 2, 3};
	multi.maps.at("farm").portals[0].width = 2;
	multi.maps.at("farm").portals[0].height = 2;
	const World round =
		serializer.loadFromString(serializer.saveToString(multi));
	EXPECT_EQ(round.maps.at("farm").front, multi.maps.at("farm").front);
	EXPECT_EQ(round.maps.at("farm").portals[0].width, 2);
	EXPECT_EQ(round.maps.at("farm").portals[0].height, 2);
}

TEST(WorldSerializerTest, SaveToStringIsDeterministic) {
	const WorldSerializer serializer;
	const World world = worldfixtures::loadMinimalWorld();
	EXPECT_EQ(serializer.saveToString(world),
		  serializer.saveToString(world));
}

TEST(WorldSerializerTest, TextRoundTripIsStable) {
	const WorldSerializer serializer;
	const std::string first =
		serializer.saveToString(worldfixtures::loadMinimalWorld());
	EXPECT_EQ(serializer.saveToString(serializer.loadFromString(first)),
		  first);
}

TEST(WorldSerializerTest, ExtraMapAndLinkSurviveRoundTrip) {
	const WorldSerializer serializer;
	const World world = serializer.loadFromString(
		serializer.saveToString(worldfixtures::loadMinimalWorld()));

	const auto& extra = world.maps.at("secret_cave");
	EXPECT_EQ(extra.kind, MapKind::Dungeon);
	EXPECT_EQ(extra.level, 3);

	const auto& town = world.maps.at("town");
	ASSERT_EQ(town.portals.size(), 1u);
	EXPECT_EQ(town.portals[0].kind, PortalKind::Link);
	EXPECT_EQ(town.portals[0].targetMap, "secret_cave");
}

TEST(WorldSerializerTest, SaveToFileReportsUnwritableOutput) {
	const World world = worldfixtures::loadMinimalWorld();
	try {
		WorldSerializer().saveToFile(world,
					     "/nonexistent-dir/world.toml");
		FAIL() << "expected FormatError";
	} catch (const FormatError& error) {
		EXPECT_EQ(error.kind(), FormatErrorKind::UnwritableOutput);
	}
}

}
