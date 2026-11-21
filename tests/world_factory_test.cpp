#include <cstddef>
#include <string>

#include "gtest/gtest.h"
#include "world_fixtures.h"
#include "world_model/FormatError.h"
#include "world_model/ScenarioLibrary.h"
#include "world_model/WorldFactory.h"
#include "world_model/WorldSerializer.h"
#include "world_model/WorldValidator.h"

namespace {

using worldfixtures::defaultCatalog;
using worldfixtures::defaultWorld;
using worldfixtures::fullLibrary;
using worldfixtures::kHeight;
using worldfixtures::kWidth;
using worldfixtures::scenarioOf;
using worldfixtures::validChoice;
using worldmodel::FormatError;
using worldmodel::FormatErrorKind;
using worldmodel::Map;
using worldmodel::MapKind;
using worldmodel::MapObject;
using worldmodel::MarkerKind;
using worldmodel::ObjectRole;
using worldmodel::Portal;
using worldmodel::PortalKind;
using worldmodel::PrefabCatalog;
using worldmodel::Scenario;
using worldmodel::ScenarioChoice;
using worldmodel::ScenarioLibrary;
using worldmodel::TileRef;
using worldmodel::TilesetDef;
using worldmodel::World;
using worldmodel::WorldFactory;
using worldmodel::WorldSerializer;

const Portal* findPortal(const Map& map, PortalKind kind) {
	for (const Portal& portal : map.portals) {
		if (portal.kind == kind) {
			return &portal;
		}
	}
	return nullptr;
}

const MapObject* findObject(const Map& map, ObjectRole role) {
	for (const MapObject& object : map.objects) {
		if (object.role == role) {
			return &object;
		}
	}
	return nullptr;
}

int countMarkers(const Map& map, MarkerKind kind) {
	int count = 0;
	for (const auto& marker : map.markers) {
		if (marker.kind == kind) {
			++count;
		}
	}
	return count;
}

template <typename Action> void expectInvalid(Action action) {
	try {
		action();
		FAIL() << "expected FormatError";
	} catch (const FormatError& error) {
		EXPECT_EQ(error.kind(), FormatErrorKind::InvalidValue);
	}
}

TEST(WorldFactoryTest, MakeDefaultBuildsEmptyFarmWithoutCabins) {
	const World empty = WorldFactory().makeDefault(
		"Primavera", worldfixtures::fullLibrary(),
		worldfixtures::defaultCatalog(), worldfixtures::validChoice());
	EXPECT_EQ(empty.name, "Primavera");
	ASSERT_EQ(empty.maps.size(), 14u);
	for (const char* id :
	     {"farm", "town", "market", "community_center"}) {
		EXPECT_TRUE(empty.maps.contains(id)) << id;
	}
	EXPECT_FALSE(empty.maps.contains("cabin_1"));
	EXPECT_TRUE(empty.maps.at("farm").objects.empty());
	for (int level = 1; level <= 10; ++level) {
		const std::string id = "dungeon_" + std::to_string(level);
		EXPECT_TRUE(empty.maps.contains(id)) << id;
	}
	const auto issues = worldmodel::WorldValidator().validate(empty);
	bool hasFarmMissing = false;
	for (const auto& issue : issues) {
		if (issue.code == worldmodel::IssueCode::MissingObject &&
		    issue.mapId == "farm") {
			hasFarmMissing = true;
		}
	}
	EXPECT_TRUE(hasFarmMissing);
	// Solo los interiores tienen su portal automatico; los externos los pone el usuario.
	EXPECT_TRUE(empty.maps.at("farm").portals.empty());
	EXPECT_TRUE(empty.maps.at("town").portals.empty());
	EXPECT_TRUE(empty.maps.at("dungeon_1").portals.empty());
	EXPECT_NE(findPortal(empty.maps.at("market"), PortalKind::InteriorExit),
		  nullptr);
	EXPECT_NE(findPortal(empty.maps.at("community_center"),
			     PortalKind::InteriorExit),
		  nullptr);
}

TEST(WorldFactoryTest, MapsHaveTheirKindAndLevel) {
	const World world = defaultWorld();
	EXPECT_EQ(world.maps.at("farm").kind, MapKind::Farm);
	EXPECT_EQ(world.maps.at("farm").level, 0);
	EXPECT_EQ(world.maps.at("community_center").kind,
		  MapKind::CommunityCenter);
	EXPECT_EQ(world.maps.at("dungeon_7").kind, MapKind::Dungeon);
	EXPECT_EQ(world.maps.at("dungeon_7").level, 7);
}

TEST(WorldFactoryTest, GridsMatchMapSize) {
	const World world = defaultWorld();
	for (const auto& [id, map] : world.maps) {
		const std::size_t cells =
			static_cast<std::size_t>(map.width) *
			static_cast<std::size_t>(map.height);
		EXPECT_EQ(map.baseCollision.size(), cells) << id;
		EXPECT_EQ(map.collision.size(), cells) << id;
		EXPECT_EQ(map.ground.size(), cells) << id;
		EXPECT_EQ(map.paths.size(), cells) << id;
	}
}

TEST(WorldFactoryTest, FarmHasCabinPrefabAndEntrancesWithoutPlayerSpawn) {
	const World world = defaultWorld();
	const Map& farm = world.maps.at("farm");
	const MapObject* cabin =
		findObject(farm, ObjectRole::Cabin);
	ASSERT_NE(cabin, nullptr);
	EXPECT_EQ(cabin->tile, (TileRef{"prefab_estructuras_granja", 0, 0}));
	EXPECT_EQ(cabin->width, 9);
	EXPECT_EQ(cabin->height, 9);
	EXPECT_TRUE(cabin->blocked);
	EXPECT_EQ(cabin->door, (worldmodel::TilePos{7, 9}));
	EXPECT_TRUE(farm.markers.empty());
	EXPECT_NE(findPortal(farm, PortalKind::TownEntrance), nullptr);
	EXPECT_NE(findPortal(farm, PortalKind::DungeonEntrance), nullptr);
}

TEST(WorldFactoryTest, TownHasBuildingsAndEntrance) {
	const World world = defaultWorld();
	const Map& town = world.maps.at("town");
	const MapObject* market = findObject(town, ObjectRole::Market);
	const MapObject* center =
		findObject(town, ObjectRole::CommunityCenter);
	ASSERT_NE(market, nullptr);
	ASSERT_NE(center, nullptr);
	EXPECT_EQ(center->tile, (TileRef{"prefab_casas_y_comercios", 8, 20}));
	EXPECT_EQ(center->width, 12);
	EXPECT_EQ(center->height, 10);
	EXPECT_TRUE(market->door.has_value());
	EXPECT_TRUE(center->door.has_value());
}

TEST(WorldFactoryTest, InteriorsHaveTheirPoints) {
	const World world = defaultWorld();
	const Map& cabin = world.maps.at("cabin_1");
	EXPECT_EQ(countMarkers(cabin, MarkerKind::Bed), 1);
	EXPECT_NE(findPortal(cabin, PortalKind::InteriorExit), nullptr);

	const Map& market = world.maps.at("market");
	EXPECT_EQ(countMarkers(market, MarkerKind::MerchantMarnie), 1);
	EXPECT_EQ(countMarkers(market, MarkerKind::MerchantClint), 1);
	EXPECT_EQ(countMarkers(market, MarkerKind::MerchantPierre), 1);
	EXPECT_EQ(countMarkers(market, MarkerKind::MerchantRobin), 1);
	EXPECT_NE(findPortal(market, PortalKind::InteriorExit), nullptr);

	const Map& center = world.maps.at("community_center");
	EXPECT_NE(findPortal(center, PortalKind::InteriorExit), nullptr);
}

TEST(WorldFactoryTest, AllPortalTargetsExistAndAreWalkable) {
	const World world = defaultWorld();
	for (const auto& [id, map] : world.maps) {
		for (const Portal& portal : map.portals) {
			const auto target =
				world.maps.find(portal.targetMap);
			ASSERT_NE(target, world.maps.end()) << id;
			EXPECT_TRUE(target->second.contains(portal.targetCell))
				<< id;
			EXPECT_TRUE(
				target->second.isWalkable(portal.targetCell))
				<< id;
		}
	}
}

TEST(WorldFactoryTest, CollisionIncludesBlockedFootprints) {
	const World world = defaultWorld();
	const Map& farm = world.maps.at("farm");
	EXPECT_FALSE(farm.baseCollision[farm.cellIndex({2, 2})]);
	EXPECT_TRUE(farm.collision[farm.cellIndex({2, 2})]);

	const Map& town = world.maps.at("town");
	const MapObject* market = findObject(town, ObjectRole::Market);
	ASSERT_NE(market, nullptr);
	for (int row = 0; row < market->height; ++row) {
		for (int column = 0; column < market->width; ++column) {
			const worldmodel::TilePos cell{
				market->cell.column + column,
				market->cell.row + row};
			EXPECT_TRUE(town.collision[town.cellIndex(cell)]);
		}
	}
}

TEST(WorldFactoryTest, WorldSurvivesRoundTrip) {
	const WorldSerializer serializer;
	const World world = defaultWorld();
	const World loaded =
		serializer.loadFromString(serializer.saveToString(world));

	EXPECT_EQ(loaded.maps.size(), world.maps.size());
	EXPECT_EQ(loaded.maps.at("farm").objects.size(),
		  world.maps.at("farm").objects.size());
	EXPECT_TRUE(loaded.maps.at("farm").markers.empty());
	EXPECT_EQ(loaded.maps.at("farm").portals.size(), 2u);
	EXPECT_EQ(loaded.maps.at("town").objects.size(), 2u);
	EXPECT_EQ(loaded.maps.at("dungeon_4").portals.size(), 2u);
	EXPECT_EQ(loaded.maps.at("dungeon_10").portals.size(), 1u);
}

TEST(WorldFactoryTest, FirstOfEachKindPicksOnePerKindAndLeavesMissingEmpty) {
	const WorldFactory factory;
	const worldmodel::ScenarioChoice full =
		factory.firstOfEachKind(worldfixtures::fullLibrary());
	EXPECT_EQ(full.farm, "farm_a");
	EXPECT_EQ(full.communityCenter, "center_a");
	EXPECT_EQ(full.dungeons[9], "cave_a");

	worldmodel::ScenarioLibrary onlyFarm;
	onlyFarm.add(scenarioOf("farm_a", MapKind::Farm));
	const worldmodel::ScenarioChoice partial =
		factory.firstOfEachKind(onlyFarm);
	EXPECT_EQ(partial.farm, "farm_a");
	EXPECT_TRUE(partial.town.empty());
	EXPECT_THROW(
		factory.makeDefault("W", onlyFarm, defaultCatalog(), partial),
		FormatError);
}

TEST(WorldFactoryTest, MissingScenarioIsInvalid) {
	const ScenarioLibrary library = fullLibrary();
	const PrefabCatalog catalog = defaultCatalog();
	ScenarioChoice choice = validChoice();
	choice.farm.clear();
	expectInvalid([&] {
		(void)WorldFactory().makeDefault("n", library, catalog,
						 choice);
	});
}

TEST(WorldFactoryTest, WrongScenarioKindIsInvalid) {
	const ScenarioLibrary library = fullLibrary();
	const PrefabCatalog catalog = defaultCatalog();
	ScenarioChoice choice = validChoice();
	choice.farm = "town_a";
	expectInvalid([&] {
		(void)WorldFactory().makeDefault("n", library, catalog,
						 choice);
	});
}

TEST(WorldFactoryTest, TooSmallScenarioIsInvalid) {
	ScenarioLibrary library = fullLibrary();
	library.add(scenarioOf("tiny_town", MapKind::Town, 3, 2));
	ScenarioChoice choice = validChoice();
	choice.town = "tiny_town";
	expectInvalid([&] {
		(void)WorldFactory().makeDefault("n", library,
						 defaultCatalog(), choice);
	});
}

TEST(WorldFactoryTest, AddExtraMapAppendsCopyWithoutPoints) {
	World world = defaultWorld();
	const Scenario extra = scenarioOf("cave_b", MapKind::Dungeon);
	WorldFactory().addExtraMap(world, extra, "cave_b");

	ASSERT_EQ(world.maps.size(), 16u);
	const Map& map = world.maps.at("cave_b");
	EXPECT_EQ(map.kind, MapKind::Dungeon);
	EXPECT_EQ(map.level, 0);
	EXPECT_EQ(map.width, kWidth);
	EXPECT_TRUE(map.objects.empty());
	EXPECT_TRUE(map.markers.empty());
	EXPECT_TRUE(map.portals.empty());
}

TEST(WorldFactoryTest, AddExtraMapRejectsReservedInvalidAndDuplicateIds) {
	World world = defaultWorld();
	const Scenario extra = scenarioOf("cave_b", MapKind::Dungeon);
	expectInvalid([&] {
		WorldFactory().addExtraMap(world, extra, "farm");
	});
	expectInvalid([&] {
		WorldFactory().addExtraMap(world, extra, "Cave B");
	});
	WorldFactory().addExtraMap(world, extra, "cave_b");
	expectInvalid([&] {
		WorldFactory().addExtraMap(world, extra, "cave_b");
	});
	EXPECT_EQ(world.maps.size(), 16u);
}

TEST(WorldFactoryTest, AddExtraMapRejectsTilesetConflict) {
	World world = defaultWorld();
	Scenario extra = scenarioOf("cave_b", MapKind::Dungeon);
	extra.tilesets["outdoors"] = TilesetDef{"other.png", 16, 16};
	expectInvalid([&] {
		WorldFactory().addExtraMap(world, extra, "cave_b");
	});
	EXPECT_EQ(world.maps.size(), 15u);
}

TEST(WorldFactoryTest, ReplaceMapScenarioRebuildsMandatoryMapAndRewires) {
	World world = defaultWorld();
	world.maps.emplace("cabin_2", world.maps.at("cabin_1"));
	world.maps.at("cabin_2").id = "cabin_2";
	const Scenario bigFarm = scenarioOf("farm_b", MapKind::Farm, 30, 20);

	WorldFactory().replaceMapScenario(world, "farm", bigFarm,
					  defaultCatalog());

	const Map& farm = world.maps.at("farm");
	EXPECT_EQ(farm.width, 30);
	EXPECT_EQ(farm.height, 20);
	ASSERT_EQ(farm.objects.size(), 2u);
	const MapObject* cabin = findObject(farm, ObjectRole::Cabin);
	ASSERT_NE(cabin, nullptr);
	EXPECT_TRUE(cabin->door.has_value());
	EXPECT_TRUE(farm.markers.empty());

	// Portales hacia/desde farm recableados contra el nuevo escenario.
	EXPECT_EQ(
		findPortal(world.maps.at("cabin_1"), PortalKind::InteriorExit)
			->targetCell,
		farm.objects[0].door);
	EXPECT_EQ(
		findPortal(world.maps.at("cabin_2"), PortalKind::InteriorExit)
			->targetCell,
		farm.objects[1].door);

	const Scenario newCabin =
		scenarioOf("cabin_b", MapKind::Cabin, 16, 12);
	WorldFactory().replaceMapScenario(world, "cabin_2", newCabin,
					  defaultCatalog());
	EXPECT_EQ(world.maps.at("cabin_2").width, 16);
	EXPECT_EQ(
		findPortal(world.maps.at("cabin_2"), PortalKind::InteriorExit)
			->targetCell,
		farm.objects[1].door);
}

TEST(WorldFactoryTest, ReplaceMapScenarioResetsExtraMap) {
	World world = defaultWorld();
	const Scenario extra = scenarioOf("cave_b", MapKind::Dungeon);
	WorldFactory().addExtraMap(world, extra, "cave_b");

	const Scenario other = scenarioOf("field_b", MapKind::Town, 9, 7);
	WorldFactory().replaceMapScenario(world, "cave_b", other);

	const Map& map = world.maps.at("cave_b");
	EXPECT_EQ(map.kind, MapKind::Town);
	EXPECT_EQ(map.level, 0);
	EXPECT_EQ(map.width, 9);
	EXPECT_EQ(map.height, 7);
	EXPECT_TRUE(map.objects.empty());
	EXPECT_TRUE(map.markers.empty());
	EXPECT_TRUE(map.portals.empty());
	EXPECT_EQ(world.maps.size(), 16u);
	EXPECT_TRUE(worldmodel::WorldValidator().validate(world).empty());
}

TEST(WorldFactoryTest, ReplaceMapScenarioRejectsWrongKindAndUnknownId) {
	World world = defaultWorld();
	const Scenario town = scenarioOf("town_b", MapKind::Town);
	expectInvalid([&] {
		WorldFactory().replaceMapScenario(world, "farm", town);
	});
	expectInvalid([&] {
		WorldFactory().replaceMapScenario(world, "big_world",
						  scenarioOf("n", MapKind::Town));
	});
	// Un escenario demasiado chico no pisa el mundo actual.
	const Scenario tinyFarm = scenarioOf("tiny_farm", MapKind::Farm, 3, 2);
	expectInvalid([&] {
		WorldFactory().replaceMapScenario(world, "farm", tinyFarm);
	});
	EXPECT_EQ(world.maps.at("farm").width, kWidth);
	EXPECT_EQ(world.maps.at("farm").objects.size(), 1u);
}

TEST(WorldFactoryTest, RemoveExtraMapDeletesOnlyExtraMaps) {
	World world = defaultWorld();
	const Scenario extra = scenarioOf("cave_b", MapKind::Dungeon);
	WorldFactory().addExtraMap(world, extra, "cave_b");
	WorldFactory().removeExtraMap(world, "cave_b");
	EXPECT_EQ(world.maps.size(), 15u);
	EXPECT_FALSE(world.maps.contains("cave_b"));

	expectInvalid([&] {
		WorldFactory().removeExtraMap(world, "farm");
	});
	expectInvalid([&] {
		WorldFactory().removeExtraMap(world, "dungeon_10");
	});
	expectInvalid([&] {
		WorldFactory().removeExtraMap(world, "no_such_map");
	});
	EXPECT_EQ(world.maps.size(), 15u);
}

TEST(WorldFactoryTest, AddCabinMapCreatesAndWiresInterior) {
	World world = defaultWorld();
	Map& farm = world.maps.at("farm");
	worldmodel::MapObject second = farm.objects[0];
	second.cell = {15, 2};
	second.door = worldmodel::TilePos{20, 9};
	farm.objects.push_back(second);
	farm.rebuildCollision();

	WorldFactory().addCabinMap(world, 2, scenarioOf("cabin_b", MapKind::Cabin));
	ASSERT_TRUE(world.maps.contains("cabin_2"));
	const Map& cabin2 = world.maps.at("cabin_2");
	EXPECT_EQ(countMarkers(cabin2, MarkerKind::Bed), 1);
	const Portal* exit = findPortal(cabin2, PortalKind::InteriorExit);
	ASSERT_NE(exit, nullptr);
	EXPECT_EQ(exit->targetMap, "farm");
	EXPECT_EQ(exit->targetCell, *second.door);
	EXPECT_TRUE(worldmodel::WorldValidator().validate(world).empty());
}

}
