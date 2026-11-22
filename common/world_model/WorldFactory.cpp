#include "world_model/WorldFactory.h"

#include <cstddef>
#include <string>
#include <utility>
#include <vector>

#include "world_model/FormatError.h"

namespace worldmodel {

World WorldFactory::makeDefault(const std::string& name,
				const ScenarioLibrary& library,
				const ScenarioChoice& choice) const {
	const auto require = [&](const std::string& id) -> const Scenario& {
		const Scenario* scenario = library.find(id);
		if (!scenario) {
			throw FormatError(FormatErrorKind::InvalidValue, "scenario");
		}
		return *scenario;
	};

	World world;
	world.formatVersion = 1;
	world.name = name;

	addMandatoryMap(world, "farm", require(choice.farm));
	addMandatoryMap(world, "town", require(choice.town));
	addMandatoryMap(world, "cabin", require(choice.cabin));
	addMandatoryMap(world, "market", require(choice.market));
	addMandatoryMap(world, "community_center",
			require(choice.communityCenter));
	for (int level = 1; level <= 10; ++level) {
		addMandatoryMap(world, rules_.dungeonId(level),
				require(choice.dungeons[level - 1]));
	}
	wirePortals(world);
	return world;
}

ScenarioChoice WorldFactory::firstOfEachKind(
	const ScenarioLibrary& library) const {
	const auto firstId = [&](MapKind kind) -> std::string {
		const std::vector<const Scenario*> options =
			library.byKind(kind);
		return options.empty() ? std::string{} : options.front()->id;
	};
	ScenarioChoice choice;
	choice.farm = firstId(MapKind::Farm);
	choice.town = firstId(MapKind::Town);
	choice.cabin = firstId(MapKind::Cabin);
	choice.market = firstId(MapKind::Market);
	choice.communityCenter = firstId(MapKind::CommunityCenter);
	choice.dungeons.fill(firstId(MapKind::Dungeon));
	return choice;
}

void WorldFactory::addExtraMap(World& world, const Scenario& scenario,
			       const std::string& id) const {
	if (rules_.find(id) != nullptr || !rules_.isValidMapId(id) ||
	    world.maps.contains(id)) {
		throw FormatError(FormatErrorKind::InvalidValue, "maps");
	}
	mergeTilesets(world, scenario);
	world.maps.emplace(id,
			   copyScenario(scenario, id, scenario.kind, 0));
}

void WorldFactory::replaceMapScenario(World& world, const std::string& mapId,
				      const Scenario& scenario) const {
	if (!world.maps.contains(mapId)) {
		throw FormatError(FormatErrorKind::InvalidValue, "maps");
	}
	// Etapa sobre una copia: si algo falla, world queda intacto.
	World staged = world;
	staged.maps.erase(mapId);
	if (rules_.find(mapId) == nullptr) {
		addExtraMap(staged, scenario, mapId);
	} else {
		addMandatoryMap(staged, mapId, scenario);
		wirePortals(staged);
	}
	world = std::move(staged);
}

void WorldFactory::removeExtraMap(World& world, const std::string& mapId) const {
	if (rules_.find(mapId) != nullptr || !world.maps.contains(mapId)) {
		throw FormatError(FormatErrorKind::InvalidValue, "maps");
	}
	world.maps.erase(mapId);
}

void WorldFactory::addMandatoryMap(World& world, const std::string& mapId,
				   const Scenario& scenario) const {
	const MapRule& rule = *rules_.find(mapId);
	if (scenario.kind != rule.kind) {
		throw FormatError(FormatErrorKind::InvalidValue, "scenario");
	}
	mergeTilesets(world, scenario);
	world.maps.emplace(mapId,
			   copyScenario(scenario, mapId, rule.kind,
					rules_.dungeonLevel(mapId).value_or(0)));
	placeMapObjects(world, mapId);
	world.maps.at(mapId).rebuildCollision();
	placeMapPoints(world, mapId);
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

TileRef WorldFactory::defaultObjectTile(const World& world) const {
	if (world.tilesets.empty()) {
		throw FormatError(FormatErrorKind::InvalidValue, "scenario");
	}
	return TileRef{world.tilesets.begin()->first, 0, 0};
}

void WorldFactory::placeMapObjects(World& world,
				   const std::string& mapId) const {
	Map& map = world.maps.at(mapId);
	switch (map.kind) {
	case MapKind::Farm:
		addObject(map, ObjectRole::Cabin, defaultObjectTile(world),
			  {2, 2}, 4, 3, true, TilePos{4, 5});
		break;
	case MapKind::Town: {
		const TileRef tile = defaultObjectTile(world);
		const TilePos marketCell{map.width / 2, 2};
		addObject(map, ObjectRole::Market, tile, marketCell, 5, 4,
			  true, TilePos{marketCell.column + 2,
					marketCell.row + 4});
		const TilePos centerCell{2, map.height - 5};
		addObject(map, ObjectRole::CommunityCenter, tile, centerCell,
			  6, 4, true,
			  TilePos{centerCell.column + 3, centerCell.row + 4});
		break;
	}
	default:
		break;
	}
}

void WorldFactory::placeMapPoints(World& world,
				  const std::string& mapId) const {
	Map& map = world.maps.at(mapId);
	switch (map.kind) {
	case MapKind::Farm:
		addMarker(map, MarkerKind::PlayerSpawn, {4, 6});
		addPortal(map, PortalKind::TownEntrance,
			  {map.width - 2, map.height / 2});
		addPortal(map, PortalKind::DungeonEntrance,
			  {map.width - 3, 2});
		break;
	case MapKind::Town:
		addPortal(map, PortalKind::FarmEntrance, {1, map.height / 2});
		break;
	case MapKind::Cabin:
		addMarker(map, MarkerKind::Bed, {2, 2});
		addPortal(map, PortalKind::InteriorExit,
			  {map.width / 2, map.height - 2});
		break;
	case MapKind::Market: {
		const MarkerKind merchants[] = {
			MarkerKind::MerchantMarnie, MarkerKind::MerchantClint,
			MarkerKind::MerchantPierre, MarkerKind::MerchantRobin};
		for (int i = 0; i < 4; ++i) {
			addMarker(map, merchants[i], {3 + 3 * i, 0});
		}
		addPortal(map, PortalKind::InteriorExit,
			  {map.width / 2, map.height - 2});
		break;
	}
	case MapKind::CommunityCenter:
		addPortal(map, PortalKind::InteriorExit,
			  {map.width / 2, map.height - 1});
		break;
	case MapKind::Dungeon:
		addPortal(map, PortalKind::DungeonExit, {2, 2});
		if (map.level < 10) {
			addPortal(map, PortalKind::StairsDown,
				  {map.width - 3, map.height - 3});
		}
		break;
	default:
		break;
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
		MapObject{role, tile, cell, width, height, blocked, door, {}});
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
		setTarget(rules_.dungeonId(level), PortalKind::DungeonExit, "farm",
			  portalCell("farm", PortalKind::DungeonEntrance));
		if (level < 10) {
			setTarget(rules_.dungeonId(level), PortalKind::StairsDown,
				  rules_.dungeonId(level + 1),
				  portalCell(rules_.dungeonId(level + 1),
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
