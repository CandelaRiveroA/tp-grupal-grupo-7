#include "world_model/WorldFactory.h"

#include <cstdlib>
#include <cstddef>
#include <limits>
#include <string>
#include <utility>
#include <vector>

#include "world_model/FormatError.h"

namespace worldmodel {

WorldFactory::WorldFactory()
	: interiorSpecs_{
		  {MapKind::Cabin, {MarkerKind::Bed, false}},
		  {MapKind::Carpentry, {MarkerKind::MerchantRobin, true}},
		  {MapKind::Blacksmith, {MarkerKind::MerchantClint, true}},
		  {MapKind::GeneralStore, {MarkerKind::MerchantPierre, true}},
		  {MapKind::Ranch, {MarkerKind::MerchantMarnie, true}},
	  } {}

World WorldFactory::makeDefault(const std::string& name,
				const ScenarioLibrary& library,
				const PrefabCatalog& catalog,
				const ScenarioChoice& choice) const {
	(void)catalog;
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
	addMandatoryMap(world, "carpentry", require(choice.carpentry));
	addMandatoryMap(world, "blacksmith", require(choice.blacksmith));
	addMandatoryMap(world, "general_store", require(choice.generalStore));
	addMandatoryMap(world, "ranch", require(choice.ranch));
	for (int level = 1; level <= kDungeonLevels; ++level) {
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
	choice.carpentry = firstId(MapKind::Carpentry);
	choice.blacksmith = firstId(MapKind::Blacksmith);
	choice.generalStore = firstId(MapKind::GeneralStore);
	choice.ranch = firstId(MapKind::Ranch);
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
				      const Scenario& scenario,
				      const PrefabCatalog& catalog) const {
	(void)catalog;
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

void WorldFactory::addCabinMap(World& world, int index,
			       const Scenario& scenario) const {
	const std::string cabinId = rules_.cabinId(index);
	if (index < kMinPlayers || index > kMaxPlayers ||
	    world.maps.contains(cabinId)) {
		throw FormatError(FormatErrorKind::InvalidValue, "maps");
	}
	addMandatoryMap(world, cabinId, scenario);
	const Map& farm = world.maps.at("farm");
	int current = 0;
	std::optional<TilePos> door;
	for (const MapObject& obj : farm.objects) {
		if (obj.role == ObjectRole::Cabin && ++current == index) {
			door = obj.door;
			break;
		}
	}
	if (door.has_value()) {
		for (Portal& portal : world.maps.at(cabinId).portals) {
			if (portal.kind == PortalKind::InteriorExit) {
				portal.targetMap = "farm";
				portal.targetCell = *door;
			}
		}
	}
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
	    scenario.ground.size() != cells ||
	    (!scenario.front.empty() && scenario.front.size() != cells)) {
		throw FormatError(FormatErrorKind::InvalidValue, "scenario");
	}
	const auto checkTiles = [&](const TileGrid& grid) {
		for (const std::optional<TileRef>& tile : grid) {
			if (tile.has_value() &&
			    scenario.tilesets.find(tile->tileset) ==
				    scenario.tilesets.end()) {
				throw FormatError(FormatErrorKind::InvalidValue,
						  "tilesets");
			}
		}
	};
	checkTiles(scenario.ground);
	checkTiles(scenario.front);

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
	if (scenario.front.empty()) {
		map.front.assign(cells, std::nullopt);
	} else {
		map.front = scenario.front;
	}
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

void WorldFactory::placeMapPoints(World& world,
				  const std::string& mapId) const {
	Map& map = world.maps.at(mapId);
	const auto it = interiorSpecs_.find(map.kind);
	if (it == interiorSpecs_.end()) {
		return;
	}
	const TilePos preferredMarker =
		it->second.centerMarker
			? TilePos{map.width / 2, map.height / 2}
			: TilePos{2, 2};
	addMarker(map, it->second.marker,
		  findWalkableNear(map, preferredMarker));
	addPortal(map, PortalKind::InteriorExit,
		  findWalkableNear(map, {map.width / 2, map.height - 2}));
}

TilePos WorldFactory::findWalkableNear(const Map& map,
				       TilePos preferred) const {
	if (map.contains(preferred) && map.isWalkable(preferred)) {
		return preferred;
	}
	int bestDist = std::numeric_limits<int>::max();
	std::optional<TilePos> best;
	for (int row = 0; row < map.height; ++row) {
		for (int col = 0; col < map.width; ++col) {
			const TilePos candidate{col, row};
			if (!map.isWalkable(candidate)) {
				continue;
			}
			const int dist =
				std::abs(col - preferred.column) +
				std::abs(row - preferred.row);
			if (dist < bestDist) {
				bestDist = dist;
				best = candidate;
			}
		}
	}
	if (best.has_value()) {
		return *best;
	}
	return map.contains(preferred) ? preferred : TilePos{0, 0};
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
	const auto doorCell = [&](const std::string& mapId,
				  ObjectRole role) -> std::optional<TilePos> {
		const auto mapIt = world.maps.find(mapId);
		if (mapIt == world.maps.end()) {
			return std::nullopt;
		}
		for (const MapObject& object : mapIt->second.objects) {
			if (object.role == role && object.door.has_value()) {
				return *object.door;
			}
		}
		return std::nullopt;
	};
	const auto cabinDoorCell =
		[&](int cabinIndex) -> std::optional<TilePos> {
		const auto farmIt = world.maps.find("farm");
		if (farmIt == world.maps.end()) {
			return std::nullopt;
		}
		int current = 0;
		for (const MapObject& object : farmIt->second.objects) {
			if (object.role == ObjectRole::Cabin &&
			    ++current == cabinIndex) {
				return object.door;
			}
		}
		return std::nullopt;
	};
	const auto setTarget = [&](const std::string& mapId, PortalKind kind,
				   const std::string& targetId,
				   std::optional<TilePos> targetCell) {
		if (!targetCell.has_value()) {
			return;
		}
		const auto mapIt = world.maps.find(mapId);
		if (mapIt == world.maps.end()) {
			return;
		}
		for (Portal& portal : mapIt->second.portals) {
			if (portal.kind == kind) {
				portal.targetMap = targetId;
				portal.targetCell = *targetCell;
				return;
			}
		}
	};

	for (int k = kMinPlayers; k <= kMaxPlayers; ++k) {
		const std::string id = rules_.cabinId(k);
		if (world.maps.contains(id)) {
			setTarget(id, PortalKind::InteriorExit, "farm",
				  cabinDoorCell(k));
		}
	}
	const std::pair<const char*, ObjectRole> shops[] = {
		{"carpentry", ObjectRole::Carpentry},
		{"blacksmith", ObjectRole::Blacksmith},
		{"general_store", ObjectRole::GeneralStore},
		{"ranch", ObjectRole::Ranch},
	};
	for (const auto& [mapId, role] : shops) {
		setTarget(mapId, PortalKind::InteriorExit, "town",
			  doorCell("town", role));
	}
}

}
