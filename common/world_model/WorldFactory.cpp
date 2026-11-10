#include "world_model/WorldFactory.h"

#include <cstddef>
#include <string>

#include "world_model/FormatError.h"

namespace worldmodel {

namespace {

std::string dungeonId(int level) {
	return "dungeon_" + std::to_string(level);
}

bool isExtraMapId(const std::string& id) {
	if (id.empty()) {
		return false;
	}
	for (const char character : id) {
		const bool valid = (character >= 'a' && character <= 'z') ||
				   (character >= '0' && character <= '9') ||
				   character == '_' || character == '-';
		if (!valid) {
			return false;
		}
	}
	return true;
}

bool isReservedMapId(const std::string& id) {
	if (id == "farm" || id == "town" || id == "cabin" ||
	    id == "market" || id == "community_center") {
		return true;
	}
	for (int level = 1; level <= 10; ++level) {
		if (id == dungeonId(level)) {
			return true;
		}
	}
	return false;
}

}

World WorldFactory::makeDefault(const std::string& slug,
				const std::string& name,
				const ScenarioLibrary& library,
				const ScenarioChoice& choice) const {
	const auto require = [&](const std::string& id,
				 MapKind kind) -> const Scenario& {
		const Scenario* scenario = id.empty() ? nullptr : library.find(id);
		if (scenario == nullptr || scenario->kind != kind) {
			throw FormatError(FormatErrorKind::InvalidValue,
					  "scenario");
		}
		return *scenario;
	};

	World world;
	world.formatVersion = 1;
	world.slug = slug;
	world.name = name;

	const auto addMap = [&](const std::string& id, MapKind kind, int level,
				const Scenario& scenario) {
		mergeTilesets(world, scenario);
		world.maps.emplace(id, copyScenario(scenario, id, kind, level));
	};

	addMap("farm", MapKind::Farm, 0,
	       require(choice.farm, MapKind::Farm));
	addMap("town", MapKind::Town, 0,
	       require(choice.town, MapKind::Town));
	addMap("cabin", MapKind::Cabin, 0,
	       require(choice.cabin, MapKind::Cabin));
	addMap("market", MapKind::Market, 0,
	       require(choice.market, MapKind::Market));
	addMap("community_center", MapKind::CommunityCenter, 0,
	       require(choice.communityCenter, MapKind::CommunityCenter));
	for (int level = 1; level <= 10; ++level) {
		addMap(dungeonId(level), MapKind::Dungeon, level,
		       require(choice.dungeons[level - 1], MapKind::Dungeon));
	}

	placeObjects(world);

	for (auto& [id, map] : world.maps) {
		map.rebuildCollision();
	}

	placePoints(world);
	wirePortals(world);
	return world;
}

void WorldFactory::addExtraMap(World& world, const Scenario& scenario,
			       const std::string& id) const {
	if (isReservedMapId(id) || !isExtraMapId(id) ||
	    world.maps.find(id) != world.maps.end()) {
		throw FormatError(FormatErrorKind::InvalidValue, "scenario");
	}
	mergeTilesets(world, scenario);
	world.maps.emplace(id,
			   copyScenario(scenario, id, scenario.kind, 0));
}

Map WorldFactory::copyScenario(const Scenario& scenario,
			       const std::string& id, MapKind kind,
			       int level) const {
	if (scenario.width <= 0 || scenario.height <= 0) {
		throw FormatError(FormatErrorKind::InvalidValue, "scenario");
	}
	const std::size_t cells = static_cast<std::size_t>(scenario.width) *
				  static_cast<std::size_t>(scenario.height);
	if (scenario.collision.size() != cells ||
	    scenario.ground.size() != cells) {
		throw FormatError(FormatErrorKind::InvalidValue, "scenario");
	}
	for (const std::optional<TileRef>& tile : scenario.ground) {
		if (tile.has_value() &&
		    scenario.tilesets.find(tile->tileset) ==
			    scenario.tilesets.end()) {
			throw FormatError(FormatErrorKind::InvalidValue,
					  "tilesets");
		}
	}

	Map map;
	map.id = id;
	map.kind = kind;
	map.level = level;
	map.width = scenario.width;
	map.height = scenario.height;
	map.tileSize = scenario.tileSize;
	map.baseCollision = scenario.collision;
	map.collision = scenario.collision;
	map.ground = scenario.ground;
	map.paths.assign(cells, std::nullopt);
	return map;
}

void WorldFactory::mergeTilesets(World& world,
				 const Scenario& scenario) const {
	for (const auto& [name, tileset] : scenario.tilesets) {
		const auto found = world.tilesets.find(name);
		if (found == world.tilesets.end()) {
			world.tilesets.emplace(name, tileset);
		} else if (!(found->second == tileset)) {
			throw FormatError(FormatErrorKind::InvalidValue,
					  "tilesets");
		}
	}
}

void WorldFactory::placeObjects(World& world) const {
	if (world.tilesets.empty()) {
		throw FormatError(FormatErrorKind::InvalidValue, "scenario");
	}
	const TileRef tile{world.tilesets.begin()->first, 0, 0};

	Map& farm = world.maps.at("farm");
	addObject(farm, ObjectRole::Cabin, tile, {2, 2}, 4, 3, true,
		  TilePos{4, 5});

	Map& town = world.maps.at("town");
	const TilePos marketCell{town.width / 2, 2};
	addObject(town, ObjectRole::Market, tile, marketCell, 5, 4, true,
		  TilePos{marketCell.column + 2, marketCell.row + 4});
	const TilePos centerCell{2, town.height - 5};
	addObject(town, ObjectRole::CommunityCenter, tile, centerCell, 6, 4,
		  true, TilePos{centerCell.column + 3, centerCell.row + 4});
}

void WorldFactory::placePoints(World& world) const {
	Map& farm = world.maps.at("farm");
	addMarker(farm, MarkerKind::PlayerSpawn, {4, 6});
	addPortal(farm, PortalKind::TownEntrance,
		  {farm.width - 2, farm.height / 2});
	addPortal(farm, PortalKind::DungeonEntrance,
		  {farm.width - 3, 2});

	Map& town = world.maps.at("town");
	addPortal(town, PortalKind::FarmEntrance, {1, town.height / 2});

	Map& cabin = world.maps.at("cabin");
	addMarker(cabin, MarkerKind::Bed, {2, 2});
	addPortal(cabin, PortalKind::InteriorExit,
		  {cabin.width / 2, cabin.height - 2});

	Map& market = world.maps.at("market");
	const MarkerKind merchants[] = {
		MarkerKind::MerchantMarnie, MarkerKind::MerchantClint,
		MarkerKind::MerchantPierre, MarkerKind::MerchantRobin};
	for (int i = 0; i < 4; ++i) {
		addMarker(market, merchants[i], {3 + 3 * i, 2});
	}
	addPortal(market, PortalKind::InteriorExit,
		  {market.width / 2, market.height - 2});

	Map& center = world.maps.at("community_center");
	addPortal(center, PortalKind::InteriorExit,
		  {center.width / 2, center.height - 2});

	for (int level = 1; level <= 10; ++level) {
		Map& dungeon = world.maps.at(dungeonId(level));
		addPortal(dungeon, PortalKind::DungeonExit, {2, 2});
		if (level < 10) {
			addPortal(dungeon, PortalKind::StairsDown,
				  {dungeon.width - 3,
				   dungeon.height - 3});
		}
	}
}

void WorldFactory::addObject(Map& map, ObjectRole role, const TileRef& tile,
			     TilePos cell, int width, int height, bool blocked,
			     std::optional<TilePos> door) const {
	if (cell.column < 0 || cell.row < 0 ||
	    cell.column + width > map.width ||
	    cell.row + height > map.height) {
		throw FormatError(FormatErrorKind::InvalidValue, "scenario");
	}
	if (door.has_value() && !map.contains(*door)) {
		throw FormatError(FormatErrorKind::InvalidValue, "scenario");
	}
	for (const MapObject& other : map.objects) {
		const bool overlaps =
			cell.column < other.cell.column + other.width &&
			other.cell.column < cell.column + width &&
			cell.row < other.cell.row + other.height &&
			other.cell.row < cell.row + height;
		if (overlaps) {
			throw FormatError(FormatErrorKind::InvalidValue,
					  "scenario");
		}
	}
	map.objects.push_back(
		MapObject{role, tile, cell, width, height, blocked, door});
}

void WorldFactory::addMarker(Map& map, MarkerKind kind,
			     TilePos cell) const {
	map.markers.push_back(Marker{kind, cell});
}

void WorldFactory::addPortal(Map& map, PortalKind kind,
			     TilePos cell) const {
	map.portals.push_back(Portal{kind, cell, "", TilePos{}});
}

void WorldFactory::wirePortals(World& world) const {
	const auto portalCell = [&](const std::string& mapId,
				    PortalKind kind) {
		const Map& map = world.maps.at(mapId);
		for (const Portal& portal : map.portals) {
			if (portal.kind == kind) {
				return portal.cell;
			}
		}
		throw FormatError(FormatErrorKind::InvalidValue, "scenario");
	};
	const auto doorCell = [&](const std::string& mapId,
				  ObjectRole role) {
		const Map& map = world.maps.at(mapId);
		for (const MapObject& object : map.objects) {
			if (object.role == role && object.door.has_value()) {
				return *object.door;
			}
		}
		throw FormatError(FormatErrorKind::InvalidValue, "scenario");
	};
	const auto setTarget = [&](const std::string& mapId, PortalKind kind,
				   const std::string& targetId,
				   TilePos targetCell) {
		Map& map = world.maps.at(mapId);
		for (Portal& portal : map.portals) {
			if (portal.kind == kind) {
				portal.targetMap = targetId;
				portal.targetCell = targetCell;
				return;
			}
		}
		throw FormatError(FormatErrorKind::InvalidValue, "scenario");
	};

	setTarget("farm", PortalKind::TownEntrance, "town",
		  portalCell("town", PortalKind::FarmEntrance));
	setTarget("town", PortalKind::FarmEntrance, "farm",
		  portalCell("farm", PortalKind::TownEntrance));
	setTarget("farm", PortalKind::DungeonEntrance, "dungeon_1",
		  portalCell("dungeon_1", PortalKind::DungeonExit));

	for (int level = 1; level <= 10; ++level) {
		setTarget(dungeonId(level), PortalKind::DungeonExit, "farm",
			  portalCell("farm", PortalKind::DungeonEntrance));
		if (level < 10) {
			setTarget(dungeonId(level), PortalKind::StairsDown,
				  dungeonId(level + 1),
				  portalCell(dungeonId(level + 1),
					     PortalKind::DungeonExit));
		}
	}

	setTarget("cabin", PortalKind::InteriorExit, "farm",
		  doorCell("farm", ObjectRole::Cabin));
	setTarget("market", PortalKind::InteriorExit, "town",
		  doorCell("town", ObjectRole::Market));
	setTarget("community_center", PortalKind::InteriorExit, "town",
		  doorCell("town", ObjectRole::CommunityCenter));
}

}
