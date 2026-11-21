#include <algorithm>
#include <cstddef>
#include <stdexcept>
#include <string>
#include <vector>

#include "gtest/gtest.h"
#include "world_fixtures.h"
#include "world_model/IssueFormatter.h"
#include "world_model/ScenarioLibrary.h"
#include "world_model/ValidationIssue.h"
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
using worldmodel::IssueCode;
using worldmodel::IssueFormatter;
using worldmodel::Map;
using worldmodel::MapKind;
using worldmodel::MapObject;
using worldmodel::Marker;
using worldmodel::MarkerKind;
using worldmodel::ObjectRole;
using worldmodel::Portal;
using worldmodel::PortalKind;
using worldmodel::Scenario;
using worldmodel::ScenarioChoice;
using worldmodel::ScenarioLibrary;
using worldmodel::TileRef;
using worldmodel::TilesetDef;
using worldmodel::ValidationIssue;
using worldmodel::World;
using worldmodel::WorldFactory;
using worldmodel::WorldSerializer;
using worldmodel::WorldValidator;

Map& mapOf(World& world, const std::string& id) {
	return world.maps.at(id);
}

MapObject& objectOf(Map& map, ObjectRole role) {
	for (MapObject& object : map.objects) {
		if (object.role == role) {
			return object;
		}
	}
	throw std::runtime_error("object not found");
}

Portal& portalOf(Map& map, PortalKind kind) {
	for (Portal& portal : map.portals) {
		if (portal.kind == kind) {
			return portal;
		}
	}
	throw std::runtime_error("portal not found");
}

Marker& markerOf(Map& map, MarkerKind kind) {
	for (Marker& marker : map.markers) {
		if (marker.kind == kind) {
			return marker;
		}
	}
	throw std::runtime_error("marker not found");
}

bool hasIssue(const World& world, IssueCode code) {
	for (const ValidationIssue& issue :
	     WorldValidator().validate(world)) {
		if (issue.code == code) {
			return true;
		}
	}
	return false;
}

void expectIssue(const World& world, IssueCode code) {
	EXPECT_TRUE(hasIssue(world, code));
}

TEST(WorldValidatorTest, DefaultWorldHasNoIssues) {
	EXPECT_TRUE(WorldValidator().validate(defaultWorld()).empty());
}

TEST(WorldValidatorTest, ExtraMapAndLinkValidateClean) {
	World world = defaultWorld();
	WorldFactory().addExtraMap(
		world, scenarioOf("cave_extra", MapKind::Farm), "cave");
	Map& farm = mapOf(world, "farm");
	farm.portals.push_back(
		Portal{PortalKind::Link, {1, 1}, "cave", {0, 0}});
	Map& cave = mapOf(world, "cave");
	cave.portals.push_back(
		Portal{PortalKind::Link, {0, 0}, "farm", {1, 1}});

	EXPECT_TRUE(WorldValidator().validate(world).empty());
}

TEST(WorldValidatorTest, FactoryWorldSurvivesRoundTripAndValidates) {
	const WorldSerializer serializer;
	const World loaded =
		serializer.loadFromString(serializer.saveToString(defaultWorld()));

	EXPECT_TRUE(WorldValidator().validate(loaded).empty());
}

TEST(WorldValidatorTest, ExtraMapWithLinkSurvivesRoundTripAndValidates) {
	World world = defaultWorld();
	WorldFactory().addExtraMap(
		world, scenarioOf("cave_extra", MapKind::Dungeon), "cave");
	mapOf(world, "farm").portals.push_back(
		Portal{PortalKind::Link, {1, 1}, "cave", {0, 0}});
	mapOf(world, "cave").portals.push_back(
		Portal{PortalKind::Link, {0, 0}, "farm", {1, 1}});

	const WorldSerializer serializer;
	const World loaded =
		serializer.loadFromString(serializer.saveToString(world));

	EXPECT_TRUE(WorldValidator().validate(loaded).empty());
}

TEST(WorldValidatorTest, ExtraDungeonAcceptsAnyLevel) {
	World world = defaultWorld();
	WorldFactory().addExtraMap(
		world, scenarioOf("cave_extra", MapKind::Dungeon), "cave");
	mapOf(world, "cave").level = 5;

	EXPECT_TRUE(WorldValidator().validate(world).empty());
}

TEST(WorldValidatorTest, ExtraNonDungeonRequiresZeroLevel) {
	World world = defaultWorld();
	WorldFactory().addExtraMap(
		world, scenarioOf("extra_farm", MapKind::Farm), "extra");
	mapOf(world, "extra").level = 3;

	expectIssue(world, IssueCode::WrongDungeonLevel);
}

TEST(WorldValidatorTest, UnsupportedVersion) {
	World world = defaultWorld();
	world.formatVersion = 2;
	expectIssue(world, IssueCode::UnsupportedVersion);
}

TEST(WorldValidatorTest, InvalidName) {
	World world = defaultWorld();
	world.name = "granja/secretos";
	expectIssue(world, IssueCode::InvalidName);
}

TEST(WorldValidatorTest, EmptyName) {
	World world = defaultWorld();
	world.name.clear();
	expectIssue(world, IssueCode::EmptyName);
}

TEST(WorldValidatorTest, UnknownTilesetInGround) {
	World world = defaultWorld();
	mapOf(world, "farm").ground[0] = TileRef{"nope", 0, 0};
	expectIssue(world, IssueCode::UnknownTileset);
}

TEST(WorldValidatorTest, MissingRequiredMap) {
	World world = defaultWorld();
	world.maps.erase("town");
	expectIssue(world, IssueCode::MissingRequiredMap);
}

TEST(WorldValidatorTest, WrongMapKind) {
	World world = defaultWorld();
	mapOf(world, "farm").kind = MapKind::Town;
	expectIssue(world, IssueCode::WrongMapKind);
}

TEST(WorldValidatorTest, WrongDungeonLevel) {
	World world = defaultWorld();
	mapOf(world, "dungeon_3").level = 2;
	expectIssue(world, IssueCode::WrongDungeonLevel);
}

TEST(WorldValidatorTest, InvalidMapId) {
	World world = defaultWorld();
	mapOf(world, "farm").id = "Farm!";

	const std::vector<ValidationIssue> issues =
		WorldValidator().validate(world);
	EXPECT_EQ(std::count_if(
			  issues.begin(), issues.end(),
			  [](const ValidationIssue& issue) {
				  return issue.code == IssueCode::InvalidMapId;
			  }),
		  1);
}

TEST(WorldValidatorTest, InvalidDimensions) {
	World world = defaultWorld();
	mapOf(world, "farm").width = 0;
	expectIssue(world, IssueCode::InvalidDimensions);
}

TEST(WorldValidatorTest, GridSizeMismatch) {
	World world = defaultWorld();
	mapOf(world, "farm").ground.pop_back();
	expectIssue(world, IssueCode::GridSizeMismatch);
}

TEST(WorldValidatorTest, CollisionOutOfSync) {
	World world = defaultWorld();
	Map& farm = mapOf(world, "farm");
	farm.collision[0] = !farm.collision[0];
	expectIssue(world, IssueCode::CollisionOutOfSync);
}

TEST(WorldValidatorTest, MissingObject) {
	World world = defaultWorld();
	mapOf(world, "town").objects.clear();
	expectIssue(world, IssueCode::MissingObject);
}

TEST(WorldValidatorTest, DuplicateObject) {
	World world = defaultWorld();
	Map& town = mapOf(world, "town");
	town.objects.push_back(objectOf(town, ObjectRole::Market));
	expectIssue(world, IssueCode::DuplicateObject);
}

TEST(WorldValidatorTest, TooManyCabinsIsDuplicateObject) {
	World world = defaultWorld();
	Map& farm = mapOf(world, "farm");
	for (int k = 2; k <= 5; ++k) {
		worldmodel::MapObject extra = farm.objects[0];
		extra.cell.column += 2;
		farm.objects.push_back(extra);
	}
	farm.rebuildCollision();
	expectIssue(world, IssueCode::DuplicateObject);
}

TEST(WorldValidatorTest, ExtraCabinMapAboveCabinCountIsInvalidMapId) {
	World world = defaultWorld();
	world.maps.emplace("cabin_2", world.maps.at("cabin_1"));
	world.maps.at("cabin_2").id = "cabin_2";
	expectIssue(world, IssueCode::InvalidMapId);
}

TEST(WorldValidatorTest, ObjectOutOfBounds) {
	World world = defaultWorld();
	objectOf(mapOf(world, "farm"), ObjectRole::Cabin).cell.column =
		kWidth;
	expectIssue(world, IssueCode::ObjectOutOfBounds);
}

TEST(WorldValidatorTest, ObjectDoorMissing) {
	World world = defaultWorld();
	objectOf(mapOf(world, "farm"), ObjectRole::Cabin).door.reset();
	expectIssue(world, IssueCode::ObjectDoorMissing);
}

TEST(WorldValidatorTest, ObjectDoorOutOfBounds) {
	World world = defaultWorld();
	objectOf(mapOf(world, "farm"), ObjectRole::Cabin).door = {-1, 0};
	expectIssue(world, IssueCode::ObjectDoorOutOfBounds);
}

TEST(WorldValidatorTest, ObjectDoorNotWalkable) {
	World world = defaultWorld();
	objectOf(mapOf(world, "farm"), ObjectRole::Cabin).door = {2, 2};
	expectIssue(world, IssueCode::ObjectDoorNotWalkable);
}

TEST(WorldValidatorTest, DuplicateBedIsRejected) {
	World world = defaultWorld();
	mapOf(world, "cabin_1")
		.markers.push_back(Marker{MarkerKind::Bed, {2, 3}});
	expectIssue(world, IssueCode::DuplicateMarker);
}

TEST(WorldValidatorTest, MissingMarker) {
	World world = defaultWorld();
	mapOf(world, "cabin_1").markers.clear();
	expectIssue(world, IssueCode::MissingMarker);
}

TEST(WorldValidatorTest, MissingMerchantMarker) {
	World world = defaultWorld();
	Map& market = mapOf(world, "market");
	market.markers.erase(
		std::remove_if(market.markers.begin(), market.markers.end(),
			       [](const Marker& marker) {
				       return marker.kind ==
					      MarkerKind::MerchantRobin;
			       }),
		market.markers.end());
	expectIssue(world, IssueCode::MissingMarker);
}

TEST(WorldValidatorTest, DuplicateMarker) {
	World world = defaultWorld();
	Map& market = mapOf(world, "market");
	market.markers.push_back(
		markerOf(market, MarkerKind::MerchantMarnie));
	expectIssue(world, IssueCode::DuplicateMarker);
}

TEST(WorldValidatorTest, UnexpectedMarker) {
	World world = defaultWorld();
	mapOf(world, "farm")
		.markers.push_back(
			Marker{MarkerKind::MerchantMarnie, {4, 6}});
	expectIssue(world, IssueCode::UnexpectedMarker);
}

TEST(WorldValidatorTest, MarkerOutOfBounds) {
	World world = defaultWorld();
	markerOf(mapOf(world, "cabin_1"), MarkerKind::Bed).cell = {
		-1, 0};
	expectIssue(world, IssueCode::MarkerOutOfBounds);
}

TEST(WorldValidatorTest, MarkerNotWalkable) {
	World world = defaultWorld();
	Map& cabin = mapOf(world, "cabin_1");
	cabin.baseCollision[0] = true;
	cabin.rebuildCollision();
	markerOf(cabin, MarkerKind::Bed).cell = {0, 0};
	expectIssue(world, IssueCode::MarkerNotWalkable);
}

TEST(WorldValidatorTest, MissingPortal) {
	World world = defaultWorld();
	mapOf(world, "cabin_1").portals.clear();
	expectIssue(world, IssueCode::MissingPortal);
}

TEST(WorldValidatorTest, DuplicatePortal) {
	World world = defaultWorld();
	Map& town = mapOf(world, "town");
	town.portals.push_back(
		portalOf(town, PortalKind::FarmEntrance));
	expectIssue(world, IssueCode::DuplicatePortal);
}

TEST(WorldValidatorTest, UnexpectedPortal) {
	World world = defaultWorld();
	mapOf(world, "dungeon_10")
		.portals.push_back(Portal{PortalKind::StairsDown,
					  {0, 0}, "dungeon_1", {0, 0}});
	expectIssue(world, IssueCode::UnexpectedPortal);
}

TEST(WorldValidatorTest, PortalOutOfBounds) {
	World world = defaultWorld();
	portalOf(mapOf(world, "farm"), PortalKind::TownEntrance).cell = {
		-1, 0};
	expectIssue(world, IssueCode::PortalOutOfBounds);
}

TEST(WorldValidatorTest, PortalNotWalkable) {
	World world = defaultWorld();
	portalOf(mapOf(world, "farm"), PortalKind::TownEntrance).cell = {
		2, 2};
	expectIssue(world, IssueCode::PortalNotWalkable);
}

TEST(WorldValidatorTest, PortalTargetMissing) {
	World world = defaultWorld();
	portalOf(mapOf(world, "farm"), PortalKind::TownEntrance)
		.targetMap = "nope";
	expectIssue(world, IssueCode::PortalTargetMissing);
}

TEST(WorldValidatorTest, LinkTargetMissing) {
	World world = defaultWorld();
	mapOf(world, "farm").portals.push_back(
		Portal{PortalKind::Link, {1, 1}, "nope", {0, 0}});
	expectIssue(world, IssueCode::PortalTargetMissing);
}

TEST(WorldValidatorTest, PortalTargetOutOfBounds) {
	World world = defaultWorld();
	portalOf(mapOf(world, "farm"), PortalKind::TownEntrance)
		.targetCell = {-1, 0};
	expectIssue(world, IssueCode::PortalTargetOutOfBounds);
}

TEST(WorldValidatorTest, PortalTargetNotWalkable) {
	World world = defaultWorld();
	portalOf(mapOf(world, "farm"), PortalKind::TownEntrance)
		.targetCell = {2, 9};
	expectIssue(world, IssueCode::PortalTargetNotWalkable);
}

TEST(WorldValidatorTest, LinkTargetNotWalkable) {
	World world = defaultWorld();
	mapOf(world, "farm").portals.push_back(
		Portal{PortalKind::Link, {1, 1}, "town", {2, 9}});
	expectIssue(world, IssueCode::PortalTargetNotWalkable);
}

TEST(WorldValidatorTest, WrongPortalTarget) {
	World world = defaultWorld();
	Portal& portal =
		portalOf(mapOf(world, "farm"), PortalKind::TownEntrance);
	portal.targetMap = "cabin_1";
	portal.targetCell = {0, 0};
	expectIssue(world, IssueCode::WrongPortalTarget);
}

TEST(WorldValidatorTest, InteriorExitMustTargetBuildingDoor) {
	World world = defaultWorld();
	Map& farm = mapOf(world, "farm");
	worldmodel::MapObject second = farm.objects[0];
	second.tile.row = 9;
	second.cell = {15, 2};
	second.door = worldmodel::TilePos{20, 9};
	farm.objects.push_back(second);
	farm.rebuildCollision();
	world.maps.emplace("cabin_2", world.maps.at("cabin_1"));
	world.maps.at("cabin_2").id = "cabin_2";
	// cabin_2 sigue apuntando a la puerta de la cabaña 1 -> WrongPortalTarget
	expectIssue(world, IssueCode::WrongPortalTarget);
	// Al apuntar a su propia puerta, el mundo con 2 cabañas valida limpio.
	portalOf(mapOf(world, "cabin_2"), PortalKind::InteriorExit)
		.targetCell = *second.door;
	EXPECT_TRUE(WorldValidator().validate(world).empty());
}

TEST(WorldValidatorTest, FixedPortalInExtraMapIsUnexpected) {
	World world = defaultWorld();
	WorldFactory().addExtraMap(
		world, scenarioOf("cave_extra", MapKind::Dungeon), "cave");
	mapOf(world, "cave").portals.push_back(
		Portal{PortalKind::TownEntrance, {0, 0}, "town", {1, 6}});
	expectIssue(world, IssueCode::UnexpectedPortal);
}

TEST(WorldValidatorTest, ReturnsAllIssues) {
	World world = defaultWorld();
	world.formatVersion = 2;
	world.name.clear();

	const std::vector<ValidationIssue> issues =
		WorldValidator().validate(world);
	EXPECT_GE(issues.size(), 2u);
}

TEST(IssueFormatterTest, FormatsWorldMapAndCellIssues) {
	const IssueFormatter formatter;
	EXPECT_EQ(formatter.format(
			  ValidationIssue{worldmodel::IssueCode::EmptyName, "",
					  {-1, -1}}),
		  "el nombre del mundo no puede ser vacio");
	EXPECT_EQ(
		formatter.format(ValidationIssue{
			worldmodel::IssueCode::MissingObject, "farm", {-1, -1}}),
		"falta un objeto requerido (mapa: farm)");
	EXPECT_EQ(formatter.format(
			  ValidationIssue{worldmodel::IssueCode::MarkerOutOfBounds,
					  "farm", {3, 4}}),
		  "marcador fuera de los limites (mapa: farm) (celda: 3, 4)");
}

TEST(IssueFormatterTest, EveryIssueCodeHasADescription) {
	const IssueFormatter formatter;
	const int last = static_cast<int>(worldmodel::IssueCode::WrongPortalTarget);
	for (int code = 0; code <= last; ++code) {
		const std::string text = formatter.format(ValidationIssue{
			static_cast<worldmodel::IssueCode>(code), "", {-1, -1}});
		EXPECT_NE(text, "problema desconocido") << "codigo " << code;
	}
}

}
