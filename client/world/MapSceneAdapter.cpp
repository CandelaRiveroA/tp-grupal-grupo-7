#include "world/MapSceneAdapter.h"

#include <string>

#include "world/ConfigError.h"
#include "world/MapAnchorAdapter.h"
#include "world/MapLayerAdapter.h"
#include "world/MapObjectAdapter.h"
#include "world/SceneKeys.h"

namespace world {

MapSceneAdapter::MapSceneAdapter(const WorldOverlay& overlay)
	: overlay_(overlay) {}

SceneDescription MapSceneAdapter::adapt(const worldmodel::World& world,
					const std::string& mapId) const {
	const std::string mapPath = std::string("maps.") + mapId;
	const auto found = world.maps.find(mapId);
	if (found == world.maps.end()) {
		throw ConfigError(ConfigErrorKind::MissingStructure, mapPath);
	}
	const worldmodel::Map& map = found->second;
	if (overlay_.scale <= 0 || map.tileSize <= 0) {
		throw ConfigError(ConfigErrorKind::InvalidValue,
				  mapPath + ".tile_size");
	}

	SceneDescription scene;
	scene.floorColor = {0, 0, 0, 255};
	scene.grid = {map.width, map.height};
	scene.cellPixelSize = map.tileSize * overlay_.scale;

	addTilesets(world, scene);
	mergeOverlayTilesets(scene);
	MapLayerAdapter().add(map, mapPath, scene);
	MapObjectAdapter(overlay_).add(map, mapPath, scene);
	addExtraElements(mapId, scene);
	MapAnchorAdapter(overlay_).add(map, scene);
	return scene;
}

void MapSceneAdapter::addTilesets(const worldmodel::World& world,
				  SceneDescription& scene) const {
	for (const auto& [name, def] : world.tilesets) {
		scene.tilesets.emplace(
			name, TileSetDef{def.image, def.cellWidth,
					 def.cellHeight});
	}
}

void MapSceneAdapter::mergeOverlayTilesets(SceneDescription& scene) const {
	for (const auto& [name, def] : overlay_.tilesets) {
		const auto existing = scene.tilesets.find(name);
		if (existing == scene.tilesets.end()) {
			scene.tilesets.emplace(name, def);
			continue;
		}
		const bool same = existing->second.image == def.image &&
				  existing->second.cellWidth == def.cellWidth &&
				  existing->second.cellHeight == def.cellHeight;
		if (!same) {
			throw ConfigError(ConfigErrorKind::InvalidValue,
					  std::string(kKeyTilesets) + "." + name);
		}
	}
}

void MapSceneAdapter::addExtraElements(const std::string& mapId,
				       SceneDescription& scene) const {
	const auto extra = overlay_.mapElements.find(mapId);
	if (extra == overlay_.mapElements.end()) {
		return;
	}
	for (const ElementPlacement& element : extra->second) {
		scene.elements.push_back(element);
	}
}

}
