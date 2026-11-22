#include "world/MapLayerAdapter.h"

#include <string>
#include <utility>

#include "world/ArtCellFit.h"
#include "world/ConfigError.h"

namespace world {

void MapLayerAdapter::add(const worldmodel::Map& map,
			  const std::string& mapPath,
			  SceneDescription& scene) const {
	addLayer(map.ground, mapPath + ".ground", map, scene,
		 LayerPass::Background);
	addLayer(map.paths, mapPath + ".paths", map, scene,
		 LayerPass::Background);
	if (!map.front.empty()) {
		addLayer(map.front, mapPath + ".front", map, scene,
			 LayerPass::Foreground);
	}
}

void MapLayerAdapter::addLayer(const worldmodel::TileGrid& grid,
			       const std::string& path,
			       const worldmodel::Map& map,
			       SceneDescription& scene,
			       const LayerPass pass) const {
	const std::size_t cellCount =
		static_cast<std::size_t>(map.width) *
		static_cast<std::size_t>(map.height);
	if (grid.size() != cellCount) {
		throw ConfigError(ConfigErrorKind::InvalidValue, path);
	}

	TileLayer layer;
	layer.size = {map.width, map.height};
	layer.cells.assign(cellCount, LayerCell{});
	layer.pass = pass;
	bool hasTiles = false;
	for (std::size_t i = 0; i < cellCount; ++i) {
		if (!grid[i].has_value()) {
			continue;
		}
		const worldmodel::TileRef& tile = *grid[i];
		const auto tileset = scene.tilesets.find(tile.tileset);
		if (tileset == scene.tilesets.end()) {
			throw ConfigError(
				ConfigErrorKind::InvalidValue,
				path + "[" + std::to_string(i) + "]");
		}
		std::size_t index = 0;
		while (index < layer.tilesets.size() &&
		       layer.tilesets[index] != tile.tileset) {
			++index;
		}
		if (index == layer.tilesets.size()) {
			// Primera vez que la capa usa esta hoja: su celda de arte
			// debe encajar exacta en la celda lógica.
			ArtCellFit().scaleFor(tileset->second, scene.cellPixelSize,
					      tile.tileset);
			layer.tilesets.push_back(tile.tileset);
		}
		layer.cells[i] = LayerCell{static_cast<int>(index),
					   {tile.column, tile.row}};
		hasTiles = true;
	}
	if (hasTiles) {
		scene.layers.push_back(std::move(layer));
	}
}

}
