#include "world/MapObjectAdapter.h"

#include <string>
#include <utility>

#include "world/ArtCellFit.h"
#include "world/ConfigError.h"
#include "world/OverlayKeys.h"
#include "world/SceneKeys.h"
#include "world_model/WorldVocabulary.h"

namespace world {

MapObjectAdapter::MapObjectAdapter(const WorldOverlay& overlay)
	: overlay_(overlay) {}

void MapObjectAdapter::add(const worldmodel::Map& map,
			   const std::string& mapPath,
			   SceneDescription& scene) const {
	for (std::size_t i = 0; i < map.objects.size(); ++i) {
		addObject(map, mapPath, i, scene);
	}
}

void MapObjectAdapter::addObject(const worldmodel::Map& map,
				 const std::string& mapPath,
				 std::size_t index,
				 SceneDescription& scene) const {
	const worldmodel::MapObject& object = map.objects[index];
	const std::string path =
		mapPath + ".objects[" + std::to_string(index) + "]";
	ElementPlacement placement = placementFor(object, path, scene);

	const auto overlay = overlay_.decorations.find(object.tile.key());
	const bool hasOverlay = overlay != overlay_.decorations.end();
	const bool hasNight = hasOverlay && overlay->second.night.has_value();
	if (hasOverlay && overlay->second.enter.has_value()) {
		// Entrable y con versión de noche a la vez no se soporta: la
		// capa frontal se dibujaría con la pieza de día.
		if (hasNight) {
			throw ConfigError(ConfigErrorKind::InvalidValue,
					  std::string(kKeyOverlayDecorations) +
						  "." + object.tile.key());
		}
		placement.enter = enterFor(*overlay->second.enter, placement,
					   map, mapPath, object.tile.key());
	}

	if (object.role != worldmodel::ObjectRole::Decoration &&
	    object.role != worldmodel::ObjectRole::Building) {
		addStructure(object, scene.elements.size(), scene);
	}

	// De noche la decoración cambia de aspecto (overlay): el elemento de
	// día se esconde y entra el de noche, en el mismo lugar del orden de
	// dibujo.
	if (hasNight) {
		placement.when = TimeOfDay::Day;
		const ElementPlacement night = nightElement(
			placement, *overlay->second.night,
			std::string(kKeyOverlayDecorations) + "." +
				object.tile.key() + "." + kKeyOverlayNightTile,
			scene);
		scene.elements.push_back(std::move(placement));
		scene.elements.push_back(night);
		return;
	}
	scene.elements.push_back(std::move(placement));
}

ElementPlacement MapObjectAdapter::placementFor(
	const worldmodel::MapObject& object, const std::string& path,
	const SceneDescription& scene) const {
	const auto tileset = scene.tilesets.find(object.tile.tileset);
	if (tileset == scene.tilesets.end()) {
		throw ConfigError(ConfigErrorKind::InvalidValue, path + ".tile");
	}
	if (object.width <= 0 || object.height <= 0) {
		throw ConfigError(ConfigErrorKind::InvalidValue, path + ".size");
	}
	const int drawScale = ArtCellFit().scaleFor(
		tileset->second, scene.cellPixelSize, object.tile.tileset);

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
	// Lo que el editor coloca "de pie" tapa a quien pasa detrás.
	placement.depthSorted = true;
	return placement;
}

ElementPlacement MapObjectAdapter::nightElement(
	const ElementPlacement& day, const NightVariant& night,
	const std::string& path, const SceneDescription& scene) const {
	const auto tileset = scene.tilesets.find(night.tileset);
	if (tileset == scene.tilesets.end()) {
		throw ConfigError(ConfigErrorKind::InvalidValue, path);
	}
	const int drawScale = ArtCellFit().scaleFor(
		tileset->second, scene.cellPixelSize, night.tileset);

	ElementPlacement element = day;
	element.tileset = night.tileset;
	element.pieceCells.clear();
	for (int row = 0; row < day.pieceSize.rows; ++row) {
		for (int column = 0; column < day.pieceSize.columns; ++column) {
			element.pieceCells.push_back(
				{night.sheetCell.column + column,
				 night.sheetCell.row + row});
		}
	}
	element.draw = {DrawMode::Single, drawScale, drawScale};
	element.when = TimeOfDay::Night;
	return element;
}

ElementEnter MapObjectAdapter::enterFor(const OverlayEnter& enter,
					const ElementPlacement& placement,
					const worldmodel::Map& map,
					const std::string& mapPath,
					const std::string& key) const {
	const std::string enterPath = std::string(kKeyOverlayDecorations) +
				      "." + key + "." + kKeyEnter;
	if (enter.lastFrontRow >= placement.pieceSize.rows) {
		throw ConfigError(ConfigErrorKind::InvalidValue,
				  enterPath + "." + kKeyEnterRows);
	}
	const worldmodel::WorldVocabulary vocabulary;
	for (const worldmodel::Marker& marker : map.markers) {
		if (vocabulary.toText(marker.kind) != enter.marker) {
			continue;
		}
		const bool inside =
			marker.cell.column >= placement.at.column &&
			marker.cell.column <
				placement.at.column + placement.span.columns &&
			marker.cell.row >= placement.at.row &&
			marker.cell.row <
				placement.at.row + placement.span.rows;
		if (!inside) {
			throw ConfigError(ConfigErrorKind::InvalidValue,
					  mapPath + "." + kKeyOverlayMarkers);
		}
		return ElementEnter{enter.firstFrontRow,
				    enter.lastFrontRow,
				    {marker.cell.column, marker.cell.row},
				    enter.facing};
	}
	throw ConfigError(ConfigErrorKind::MissingStructure,
			  mapPath + "." + kKeyOverlayMarkers);
}

void MapObjectAdapter::addStructure(const worldmodel::MapObject& object,
				    std::size_t elementIndex,
				    SceneDescription& scene) const {
	SceneStructure structure;
	structure.role = worldmodel::WorldVocabulary().toText(object.role);
	structure.at = {object.cell.column, object.cell.row};
	structure.span = {object.width, object.height};
	if (object.door.has_value()) {
		structure.door = GridCell{object.door->column, object.door->row};
	}
	structure.elementIndex = elementIndex;
	const auto behavior = overlay_.roles.find(structure.role);
	if (behavior != overlay_.roles.end()) {
		structure.action = behavior->second.action;
		structure.target = behavior->second.target;
	}
	scene.structures.push_back(std::move(structure));
}

}
