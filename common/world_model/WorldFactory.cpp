#include "world_model/WorldFactory.h"

#include <algorithm>
#include <cstddef>
#include <string>
#include <utility>
#include <vector>

#include "world_model/FormatError.h"

namespace worldmodel {

WorldFactory::WorldFactory()
	: objectPlacers_{
		  {MapKind::Farm, &WorldFactory::placeFarmObjects},
		  {MapKind::Town, &WorldFactory::placeTownObjects},
	  },
	  pointPlacers_{
		  {MapKind::Cabin, &WorldFactory::placeCabinPoints},
		  {MapKind::Market, &WorldFactory::placeMarketPoints},
		  {MapKind::CommunityCenter,
		   &WorldFactory::placeCommunityCenterPoints},
	  } {}

World WorldFactory::makeDefault(const std::string& name,
				const ScenarioLibrary& library,
				const PrefabCatalog& catalog,
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

	addMandatoryMap(world, "farm", require(choice.farm), catalog);
	addMandatoryMap(world, "town", require(choice.town), catalog);
	addMandatoryMap(world, "market", require(choice.market), catalog);
	addMandatoryMap(world, "community_center",
			require(choice.communityCenter), catalog);
	for (int level = 1; level <= kDungeonLevels; ++level) {
		addMandatoryMap(world, rules_.dungeonId(level),
				require(choice.dungeons[level - 1]), catalog);
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
				      const Scenario& scenario,
				      const PrefabCatalog& catalog) const {
	if (!world.maps.contains(mapId)) {
		throw FormatError(FormatErrorKind::InvalidValue, "maps");
	}
	// Etapa sobre una copia: si algo falla, world queda intacto.
	World staged = world;
	staged.maps.erase(mapId);
	if (rules_.find(mapId) == nullptr) {
		addExtraMap(staged, scenario, mapId);
	} else {
		addMandatoryMap(staged, mapId, scenario, catalog);
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
	addMandatoryMap(world, cabinId, scenario, {});
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
				   const Scenario& scenario,
				   const PrefabCatalog& catalog) const {
	const MapRule& rule = *rules_.find(mapId);
	if (scenario.kind != rule.kind) {
		throw FormatError(FormatErrorKind::InvalidValue, "scenario");
	}
	mergeTilesets(world, scenario);
	world.maps.emplace(mapId,
			   copyScenario(scenario, mapId, rule.kind,
					rules_.dungeonLevel(mapId).value_or(0)));
	placeMapObjects(world, mapId, catalog);
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

void WorldFactory::mergePrefabTileset(World& world,
				      const PrefabCatalog& catalog,
				      const std::string& tilesetId) const {
	const auto catIt = catalog.tilesets().find(tilesetId);
	if (catIt == catalog.tilesets().end()) {
		throw FormatError(FormatErrorKind::InvalidValue, "tilesets");
	}
	const auto found = world.tilesets.find(tilesetId);
	if (found == world.tilesets.end()) {
		world.tilesets.emplace(tilesetId, catIt->second);
	} else if (!(found->second == catIt->second)) {
		throw FormatError(FormatErrorKind::InvalidValue, "tilesets");
	}
}

TileRef WorldFactory::defaultObjectTile(const World& world) const {
	if (world.tilesets.empty()) {
		throw FormatError(FormatErrorKind::InvalidValue, "scenario");
	}
	return TileRef{world.tilesets.begin()->first, 0, 0};
}

void WorldFactory::placeMapObjects(World& world, const std::string& mapId,
				   const PrefabCatalog& catalog) const {
	Map& map = world.maps.at(mapId);
	const auto it = objectPlacers_.find(map.kind);
	if (it != objectPlacers_.end()) {
		(this->*(it->second))(world, map, catalog);
	}
}

void WorldFactory::placeFarmObjects(World& world, Map& map,
				    const PrefabCatalog& catalog) const {
	int cabinCount = 0;
	for (int k = kMinPlayers; k <= kMaxPlayers; ++k) {
		if (world.maps.contains(rules_.cabinId(k))) {
			cabinCount = k;
		}
	}
	const PrefabDef* cabinPrefab = catalog.findByRole(ObjectRole::Cabin);
	const int cabinWidth =
		cabinPrefab != nullptr ? cabinPrefab->width : kFallbackCabinWidth;
	for (int k = 1; k <= cabinCount; ++k) {
		const TilePos cell{
			kFarmCabinOrigin.column +
				(k - 1) * (cabinWidth + kFarmCabinGap),
			kFarmCabinOrigin.row};
		placeBuilding(world, map, ObjectRole::Cabin, cell,
			      kFallbackCabinWidth, kFallbackCabinHeight,
			      kFallbackCabinDoor, catalog);
	}
}

void WorldFactory::placeTownObjects(World& world, Map& map,
				    const PrefabCatalog& catalog) const {
	const TilePos marketCell{map.width / 2, 2};
	placeBuilding(world, map, ObjectRole::Market, marketCell,
		      kFallbackMarketWidth, kFallbackMarketHeight,
		      kFallbackMarketDoor, catalog);
	const PrefabDef* centerPrefab =
		catalog.findByRole(ObjectRole::CommunityCenter);
	const int centerHeight = centerPrefab != nullptr
					   ? centerPrefab->height
					   : kFallbackCenterHeight;
	const TilePos centerCell{2, map.height - centerHeight - 1};
	placeBuilding(world, map, ObjectRole::CommunityCenter, centerCell,
		      kFallbackCenterWidth, kFallbackCenterHeight,
		      kFallbackCenterDoor, catalog);
}

void WorldFactory::placeBuilding(World& world, Map& map, ObjectRole role,
				 TilePos cell, int defaultWidth,
				 int defaultHeight, TilePos defaultDoorOffset,
				 const PrefabCatalog& catalog) const {
	const PrefabDef* prefab = catalog.findByRole(role);
	if (prefab != nullptr) {
		mergePrefabTileset(world, catalog, prefab->tile.tileset);
		const TilePos doorOffset = prefab->door.value_or(
			TilePos{prefab->width / 2, prefab->height});
		const TilePos door{cell.column + doorOffset.column,
				   cell.row + doorOffset.row};
		const bool blocked = std::any_of(
			prefab->collisionGrid.begin(),
			prefab->collisionGrid.end(),
			[](bool cellBlocked) { return cellBlocked; });
		addObject(map, role, prefab->tile, cell, prefab->width,
			  prefab->height, blocked, door, prefab->collisionGrid);
		return;
	}
	const TilePos door{cell.column + defaultDoorOffset.column,
			   cell.row + defaultDoorOffset.row};
	addObject(map, role, defaultObjectTile(world), cell, defaultWidth,
		  defaultHeight, true, door, {});
}

void WorldFactory::placeMapPoints(World& world,
				  const std::string& mapId) const {
	Map& map = world.maps.at(mapId);
	const auto it = pointPlacers_.find(map.kind);
	if (it != pointPlacers_.end()) {
		(this->*(it->second))(map);
	}
}

void WorldFactory::placeCabinPoints(Map& map) const {
	addMarker(map, MarkerKind::Bed, {2, 2});
	addPortal(map, PortalKind::InteriorExit,
		  {map.width / 2, map.height - 2});
}

void WorldFactory::placeMarketPoints(Map& map) const {
	const MarkerKind merchants[] = {
		MarkerKind::MerchantMarnie, MarkerKind::MerchantClint,
		MarkerKind::MerchantPierre, MarkerKind::MerchantRobin};
	for (int i = 0; i < 4; ++i) {
		addMarker(map, merchants[i], {3 + 3 * i, 0});
	}
	addPortal(map, PortalKind::InteriorExit,
		  {map.width / 2, map.height - 2});
}

void WorldFactory::placeCommunityCenterPoints(Map& map) const {
	addPortal(map, PortalKind::InteriorExit,
		  {map.width / 2, map.height - 1});
}

void WorldFactory::addObject(Map& map, ObjectRole role, const TileRef& tile,
			     TilePos cell, int width, int height, bool blocked,
			     std::optional<TilePos> door,
			     std::vector<bool> collisionGrid) const {
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
	map.objects.push_back(MapObject{role, tile, cell, width, height,
					blocked, door, std::move(collisionGrid)});
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
				  ObjectRole role) {
		const Map& map = world.maps.at(mapId);
		for (const MapObject& object : map.objects) {
			if (object.role == role && object.door.has_value()) {
				return *object.door;
			}
		}
		throw FormatError(FormatErrorKind::InvalidValue, "scenario");
	};
	const auto cabinDoorCell = [&](int cabinIndex) {
		const Map& farm = world.maps.at("farm");
		int current = 0;
		for (const MapObject& object : farm.objects) {
			if (object.role == ObjectRole::Cabin) {
				++current;
				if (current == cabinIndex &&
				    object.door.has_value()) {
					return *object.door;
				}
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

	for (int k = kMinPlayers; k <= kMaxPlayers; ++k) {
		const std::string id = rules_.cabinId(k);
		if (world.maps.contains(id)) {
			setTarget(id, PortalKind::InteriorExit, "farm",
				  cabinDoorCell(k));
		}
	}
	setTarget("market", PortalKind::InteriorExit, "town",
		  doorCell("town", ObjectRole::Market));
	setTarget("community_center", PortalKind::InteriorExit, "town",
		  doorCell("town", ObjectRole::CommunityCenter));
}

}
