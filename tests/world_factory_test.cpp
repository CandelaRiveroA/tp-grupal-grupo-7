#include <algorithm>
#include <cstddef>
#include <string>

#include "gtest/gtest.h"
#include "world_fixtures.h"
#include "world_model/FormatError.h"
#include "world_model/MapRules.h"
#include "world_model/ScenarioLibrary.h"
#include "world_model/WorldFactory.h"
#include "world_model/WorldSerializer.h"
#include "world_model/WorldValidator.h"

namespace {

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
		worldfixtures::validChoice());
	EXPECT_EQ(empty.name, "Primavera");
	ASSERT_EQ(empty.maps.size(), 17u);
	for (const char* id : {"farm", "town", "carpentry", "blacksmith",
			       "general_store", "ranch", "barn"}) {
		EXPECT_TRUE(empty.maps.contains(id)) << id;
	}
	EXPECT_FALSE(empty.maps.contains("cabin_1"));
	EXPECT_TRUE(empty.maps.at("farm").objects.empty());
	EXPECT_TRUE(empty.maps.at("town").objects.empty());
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
	// Solo los interiores tienen su portal automatico; sin edificios en town quedan sin targetMap.
	EXPECT_TRUE(empty.maps.at("farm").portals.empty());
	EXPECT_TRUE(empty.maps.at("town").portals.empty());
	EXPECT_TRUE(empty.maps.at("dungeon_1").portals.empty());
	for (const char* id :
	     {"carpentry", "blacksmith", "general_store", "ranch", "barn"}) {
		const Portal* exit =
			findPortal(empty.maps.at(id), PortalKind::InteriorExit);
		ASSERT_NE(exit, nullptr) << id;
		EXPECT_TRUE(exit->targetMap.empty()) << id;
	}
}

TEST(WorldFactoryTest, MapsHaveTheirKindAndLevel) {
	const World world = defaultWorld();
	EXPECT_EQ(world.maps.at("farm").kind, MapKind::Farm);
	EXPECT_EQ(world.maps.at("farm").level, 0);
	EXPECT_EQ(world.maps.at("carpentry").kind, MapKind::Carpentry);
	EXPECT_EQ(world.maps.at("blacksmith").kind, MapKind::Blacksmith);
	EXPECT_EQ(world.maps.at("general_store").kind, MapKind::GeneralStore);
	EXPECT_EQ(world.maps.at("ranch").kind, MapKind::Ranch);
	EXPECT_EQ(world.maps.at("barn").kind, MapKind::Barn);
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
	for (const ObjectRole role :
	     {ObjectRole::Carpentry, ObjectRole::Blacksmith,
	      ObjectRole::GeneralStore, ObjectRole::Ranch}) {
		const MapObject* shop = findObject(town, role);
		ASSERT_NE(shop, nullptr);
		EXPECT_TRUE(shop->door.has_value());
	}
	const MapObject* center =
		findObject(town, ObjectRole::CommunityCenter);
	ASSERT_NE(center, nullptr);
	EXPECT_EQ(center->tile, (TileRef{"prefab_casas_y_comercios", 8, 20}));
	EXPECT_EQ(center->width, 12);
	EXPECT_EQ(center->height, 10);
	EXPECT_FALSE(center->door.has_value());
}

TEST(WorldFactoryTest, InteriorsHaveTheirPoints) {
	const World world = defaultWorld();
	const Map& cabin = world.maps.at("cabin_1");
	EXPECT_EQ(countMarkers(cabin, MarkerKind::Bed), 1);
	EXPECT_NE(findPortal(cabin, PortalKind::InteriorExit), nullptr);

	const struct {
		const char* id;
		MarkerKind merchant;
	} shops[] = {
		{"carpentry", MarkerKind::MerchantRobin},
		{"blacksmith", MarkerKind::MerchantClint},
		{"general_store", MarkerKind::MerchantPierre},
		{"ranch", MarkerKind::MerchantMarnie},
	};
	for (const auto& entry : shops) {
		const Map& shop = world.maps.at(entry.id);
		EXPECT_EQ(countMarkers(shop, entry.merchant), 1) << entry.id;
		EXPECT_EQ(shop.markers.size(), 1u) << entry.id;
		EXPECT_NE(findPortal(shop, PortalKind::InteriorExit), nullptr)
			<< entry.id;
	}
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
	const MapObject* carpentry = findObject(town, ObjectRole::Carpentry);
	ASSERT_NE(carpentry, nullptr);
	for (int row = 0; row < carpentry->height; ++row) {
		for (int column = 0; column < carpentry->width; ++column) {
			const worldmodel::TilePos cell{
				carpentry->cell.column + column,
				carpentry->cell.row + row};
			const bool expected = carpentry->collisionGrid
				[static_cast<std::size_t>(row) *
					 static_cast<std::size_t>(
						 carpentry->width) +
				 static_cast<std::size_t>(column)];
			EXPECT_EQ(town.collision[town.cellIndex(cell)],
				  expected);
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
	EXPECT_EQ(loaded.maps.at("town").objects.size(), 5u);
	EXPECT_EQ(loaded.maps.at("dungeon_4").portals.size(), 2u);
	EXPECT_EQ(loaded.maps.at("dungeon_10").portals.size(), 1u);
}

TEST(WorldFactoryTest, FirstOfEachKindPicksOnePerKindAndLeavesMissingEmpty) {
	const WorldFactory factory;
	const worldmodel::ScenarioChoice full =
		factory.firstOfEachKind(worldfixtures::fullLibrary());
	EXPECT_EQ(full.farm, "farm_a");
	EXPECT_EQ(full.carpentry, "carpentry_a");
	EXPECT_EQ(full.blacksmith, "blacksmith_a");
	EXPECT_EQ(full.generalStore, "general_store_a");
	EXPECT_EQ(full.ranch, "ranch_a");
	EXPECT_EQ(full.barn, "barn_a");
	EXPECT_EQ(full.dungeons[9], "cave_a");

	worldmodel::ScenarioLibrary onlyFarm;
	onlyFarm.add(scenarioOf("farm_a", MapKind::Farm));
	const worldmodel::ScenarioChoice partial =
		factory.firstOfEachKind(onlyFarm);
	EXPECT_EQ(partial.farm, "farm_a");
	EXPECT_TRUE(partial.town.empty());
	EXPECT_THROW(factory.makeDefault("W", onlyFarm, partial), FormatError);
}

TEST(WorldFactoryTest, MissingScenarioIsInvalid) {
	const ScenarioLibrary library = fullLibrary();
	ScenarioChoice choice = validChoice();
	choice.farm.clear();
	expectInvalid([&] {
		(void)WorldFactory().makeDefault("n", library, choice);
	});
}

TEST(WorldFactoryTest, WrongScenarioKindIsInvalid) {
	const ScenarioLibrary library = fullLibrary();
	ScenarioChoice choice = validChoice();
	choice.farm = "town_a";
	expectInvalid([&] {
		(void)WorldFactory().makeDefault("n", library, choice);
	});
}

TEST(WorldFactoryTest, ZeroDimensionScenarioIsInvalid) {
	ScenarioLibrary library = fullLibrary();
	Scenario bad = scenarioOf("bad_town", MapKind::Town, 3, 2);
	bad.width = 0;
	library.add(bad);
	ScenarioChoice choice = validChoice();
	choice.town = "bad_town";
	expectInvalid([&] {
		(void)WorldFactory().makeDefault("n", library, choice);
	});
}

TEST(WorldFactoryTest, PlaceMapPointsSnapsToNearestWalkableWhenBlocked) {
	ScenarioLibrary library = fullLibrary();
	Scenario blocked = scenarioOf("blocked_carpentry", MapKind::Carpentry,
				      4, 4);
	blocked.collision.assign(16, true);
	blocked.collision[0] = false;  // solo {0, 0} transitable
	library.add(blocked);
	ScenarioChoice choice = validChoice();
	choice.carpentry = "blocked_carpentry";

	const World world =
		WorldFactory().makeDefault("n", library, choice);
	const Map& carpentry = world.maps.at("carpentry");
	ASSERT_EQ(carpentry.markers.size(), 1u);
	EXPECT_EQ(carpentry.markers[0].cell, (worldmodel::TilePos{0, 0}));
	const Portal* exit = findPortal(carpentry, PortalKind::InteriorExit);
	ASSERT_NE(exit, nullptr);
	EXPECT_EQ(exit->cell, (worldmodel::TilePos{0, 0}));
}

TEST(WorldFactoryTest, AddExtraMapAppendsCopyWithoutPoints) {
	World world = defaultWorld();
	const Scenario extra = scenarioOf("cave_b", MapKind::Dungeon);
	WorldFactory().addExtraMap(world, extra, "cave_b");

	ASSERT_EQ(world.maps.size(), 19u);
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
	EXPECT_EQ(world.maps.size(), 19u);
}

TEST(WorldFactoryTest, AddExtraMapRejectsTilesetConflict) {
	World world = defaultWorld();
	Scenario extra = scenarioOf("cave_b", MapKind::Dungeon);
	extra.tilesets["outdoors"] = TilesetDef{"other.png", 16, 16};
	expectInvalid([&] {
		WorldFactory().addExtraMap(world, extra, "cave_b");
	});
	EXPECT_EQ(world.maps.size(), 18u);
}

TEST(WorldFactoryTest, ReplaceMapScenarioRebuildsMandatoryMapAndRewires) {
	World world = defaultWorld();
	const MapObject* townCarpentry =
		findObject(world.maps.at("town"), ObjectRole::Carpentry);
	ASSERT_NE(townCarpentry, nullptr);
	ASSERT_TRUE(townCarpentry->door.has_value());

	const Scenario newCarpentry =
		scenarioOf("carpentry_b", MapKind::Carpentry, 16, 12);
	WorldFactory().replaceMapScenario(world, "carpentry", newCarpentry);
	EXPECT_EQ(world.maps.at("carpentry").width, 16);
	const Portal* carpentryExit =
		findPortal(world.maps.at("carpentry"), PortalKind::InteriorExit);
	ASSERT_NE(carpentryExit, nullptr);
	EXPECT_EQ(carpentryExit->targetMap, "town");
	EXPECT_EQ(carpentryExit->targetCell, *townCarpentry->door);

	// Reemplazar farm limpia edificios sin fallar aunque exista cabin_1.
	const Scenario bigFarm = scenarioOf("farm_b", MapKind::Farm, 30, 20);
	WorldFactory().replaceMapScenario(world, "farm", bigFarm);
	const Map& farm = world.maps.at("farm");
	EXPECT_EQ(farm.width, 30);
	EXPECT_EQ(farm.height, 20);
	EXPECT_TRUE(farm.objects.empty());
	EXPECT_TRUE(farm.markers.empty());
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
	EXPECT_EQ(world.maps.size(), 19u);
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
	// Un escenario invalido no pisa el mundo actual.
	Scenario badFarm = scenarioOf("bad_farm", MapKind::Farm, 3, 2);
	badFarm.width = 0;
	expectInvalid([&] {
		WorldFactory().replaceMapScenario(world, "farm", badFarm);
	});
	EXPECT_EQ(world.maps.at("farm").width, kWidth);
	EXPECT_EQ(world.maps.at("farm").objects.size(), 2u);
}

TEST(WorldFactoryTest, RemoveExtraMapDeletesOnlyExtraMaps) {
	World world = defaultWorld();
	const Scenario extra = scenarioOf("cave_b", MapKind::Dungeon);
	WorldFactory().addExtraMap(world, extra, "cave_b");
	WorldFactory().removeExtraMap(world, "cave_b");
	EXPECT_EQ(world.maps.size(), 18u);
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
	EXPECT_EQ(world.maps.size(), 18u);
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

TEST(WorldFactoryTest, WirePortalsClearsStaleExitsWhenBuildingMissingOrDuplicated) {
	World world = defaultWorld();
	Map& farm = world.maps.at("farm");
	farm.objects.erase(
		std::remove_if(farm.objects.begin(), farm.objects.end(),
			       [](const MapObject& obj) {
				       return obj.role == ObjectRole::Barn;
			       }),
		farm.objects.end());

	Map& town = world.maps.at("town");
	const MapObject* carpentry = findObject(town, ObjectRole::Carpentry);
	ASSERT_NE(carpentry, nullptr);
	MapObject duplicateCarpentry = *carpentry;
	duplicateCarpentry.cell = {2, 10};
	duplicateCarpentry.door = worldmodel::TilePos{4, 15};
	town.objects.push_back(duplicateCarpentry);

	WorldFactory().wirePortals(world);

	const Portal* barnExit =
		findPortal(world.maps.at("barn"), PortalKind::InteriorExit);
	ASSERT_NE(barnExit, nullptr);
	EXPECT_TRUE(barnExit->targetMap.empty());

	const Portal* carpentryExit = findPortal(world.maps.at("carpentry"),
						 PortalKind::InteriorExit);
	ASSERT_NE(carpentryExit, nullptr);
	EXPECT_TRUE(carpentryExit->targetMap.empty());

	town.objects.pop_back();
	WorldFactory().wirePortals(world);
	EXPECT_EQ(carpentryExit->targetMap, "town");
	EXPECT_EQ(carpentryExit->targetCell, *carpentry->door);
}

TEST(WorldFactoryTest, MapRulesQueriesExposeCanonicalTopology) {
	const worldmodel::MapRules rules;
	EXPECT_EQ(rules.requiredHostMap(ObjectRole::Cabin), "farm");
	EXPECT_EQ(rules.requiredHostMap(ObjectRole::Barn), "farm");
	EXPECT_EQ(rules.requiredHostMap(ObjectRole::Carpentry), "town");
	EXPECT_EQ(rules.requiredHostMap(ObjectRole::CommunityCenter), "town");
	EXPECT_FALSE(rules.requiredHostMap(ObjectRole::Decoration).has_value());

	const std::vector<std::string> interiors = rules.fixedInteriorIds();
	EXPECT_EQ(interiors, (std::vector<std::string>{"carpentry", "blacksmith",
						      "general_store", "ranch",
						      "barn"}));
	EXPECT_TRUE(rules.isInteriorHost("farm"));
	EXPECT_TRUE(rules.isInteriorHost("town"));
	EXPECT_FALSE(rules.isInteriorHost("barn"));
	EXPECT_FALSE(rules.isInteriorHost("dungeon_1"));

	const std::vector<std::string> required = rules.requiredMapIds(2);
	EXPECT_NE(std::find(required.begin(), required.end(), "barn"),
		  required.end());
	EXPECT_NE(std::find(required.begin(), required.end(), "cabin_2"),
		  required.end());
	EXPECT_EQ(std::find(required.begin(), required.end(), "cabin_3"),
		  required.end());
}

}
