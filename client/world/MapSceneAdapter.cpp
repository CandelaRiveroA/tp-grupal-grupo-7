#include "world/MapSceneAdapter.h"

#include <string>

#include "world/ConfigError.h"
#include "world/SceneKeys.h"

namespace world {

MapSceneAdapter::MapSceneAdapter(int scale) : scale_(scale) {}

SceneDescription MapSceneAdapter::adapt(const worldmodel::World& world,
					const std::string& mapId) const {
	const std::string mapPath = std::string("maps.") + mapId;
	const auto found = world.maps.find(mapId);
	if (found == world.maps.end()) {
		throw ConfigError(ConfigErrorKind::MissingStructure, mapPath);
	}
	const worldmodel::Map& map = found->second;
	if (scale_ <= 0 || map.tileSize <= 0) {
		throw ConfigError(ConfigErrorKind::InvalidValue,
				  mapPath + ".tile_size");
	}

	SceneDescription scene;
	scene.floorColor = {0, 0, 0, 255};
	scene.grid = {map.width, map.height};
	scene.cellPixelSize = map.tileSize * scale_;

	addTilesets(world, scene);
	addLayers(map, mapPath, scene);
	addObjects(map, mapPath, scene);
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

void MapSceneAdapter::addLayers(const worldmodel::Map& map,
				const std::string& mapPath,
				SceneDescription& scene) const {
	addLayer(map.ground, mapPath + ".ground", map, scene);
	addLayer(map.paths, mapPath + ".paths", map, scene);
}

void MapSceneAdapter::addLayer(const worldmodel::TileGrid& grid,
			       const std::string& path,
			       const worldmodel::Map& map,
			       SceneDescription& scene) const {
	const std::size_t cellCount =
		static_cast<std::size_t>(map.width) *
		static_cast<std::size_t>(map.height);
	if (grid.size() != cellCount) {
		throw ConfigError(ConfigErrorKind::InvalidValue, path);
	}

	TileLayer layer;
	layer.size = {map.width, map.height};
	layer.cells.assign(cellCount, LayerCell{});
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
			drawScaleFor(tileset->second, scene.cellPixelSize,
				     std::string(kKeyTilesets) + "." +
					     tile.tileset + "." + kKeyCell);
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

void MapSceneAdapter::addObjects(const worldmodel::Map& map,
				 const std::string& mapPath,
				 SceneDescription& scene) const {
	// El orden del TOML es el orden de dibujo.
	for (std::size_t i = 0; i < map.objects.size(); ++i) {
		const worldmodel::MapObject& object = map.objects[i];
		const std::string path =
			mapPath + ".objects[" + std::to_string(i) + "]";

		const auto tileset = scene.tilesets.find(object.tile.tileset);
		if (tileset == scene.tilesets.end()) {
			throw ConfigError(ConfigErrorKind::InvalidValue,
					  path + ".tile");
		}
		if (object.width <= 0 || object.height <= 0) {
			throw ConfigError(ConfigErrorKind::InvalidValue,
					  path + ".size");
		}
		const int drawScale = drawScaleFor(
			tileset->second, scene.cellPixelSize,
			std::string(kKeyTilesets) + "." + object.tile.tileset +
				"." + kKeyCell);

		ElementPlacement placement;
		placement.tileset = object.tile.tileset;
		placement.pieceSize = {object.width, object.height};
		// Bloque contiguo de la hoja desde el tile de origen (el mismo
		// recorte que hace el editor), fila por fila.
		for (int row = 0; row < object.height; ++row) {
			for (int column = 0; column < object.width; ++column) {
				placement.pieceCells.push_back(
					{object.tile.column + column,
					 object.tile.row + row});
			}
		}
		placement.draw = {DrawMode::Single, drawScale, drawScale};
		placement.at = {object.cell.column, object.cell.row};
		placement.span = {object.width, object.height};
		scene.elements.push_back(std::move(placement));
	}
}

int MapSceneAdapter::drawScaleFor(const TileSetDef& tileset,
				  int cellPixelSize,
				  const std::string& path) const {
	const bool exact = tileset.cellWidth > 0 && tileset.cellHeight > 0 &&
			   cellPixelSize % tileset.cellWidth == 0 &&
			   cellPixelSize % tileset.cellHeight == 0 &&
			   cellPixelSize / tileset.cellWidth ==
				   cellPixelSize / tileset.cellHeight;
	if (!exact) {
		throw ConfigError(ConfigErrorKind::InvalidValue, path);
	}
	return cellPixelSize / tileset.cellWidth;
}

}
