#include "world_model/WorldTomlWriter.h"

#include <algorithm>
#include <optional>
#include <sstream>
#include <utility>

#include <toml++/toml.hpp>

#include "world_model/TileCodec.h"
#include "world_model/FieldWriter.h"

namespace worldmodel {

std::string WorldTomlWriter::write(const World& world) const {
	toml::table table;
	table.insert("format_version", world.formatVersion);
	table.insert("world", writeWorld(world));
	table.insert("tilesets", FieldWriter::writeTilesets(world.tilesets));

	toml::table maps;
	for (const auto& [id, map] : world.maps) {
		maps.insert(id, writeMap(map));
	}
	table.insert("maps", std::move(maps));

	std::ostringstream stream;
	stream << table;
	return stream.str();
}

toml::table WorldTomlWriter::writeWorld(const World& world) const {
	toml::table table;
	table.insert("name", world.name);
	return table;
}

toml::table WorldTomlWriter::writeMap(const Map& map) const {
	toml::table table;
	table.insert("kind", vocabulary_.toText(map.kind));
	if (map.kind == MapKind::Dungeon) {
		table.insert("level", map.level);
	}
	table.insert("width", map.width);
	table.insert("height", map.height);
	table.insert("tile_size", map.tileSize);
	table.insert("base_collision",
		     FieldWriter::writeBooleans(map.baseCollision));
	table.insert("collision",
		     FieldWriter::writeBooleans(map.collision));
	table.insert("ground", FieldWriter::writeGrid(map.ground));

	const bool hasPaths =
		std::any_of(map.paths.begin(), map.paths.end(),
			    [](const std::optional<TileRef>& tile) {
				    return tile.has_value();
			    });
	if (hasPaths) {
		table.insert("paths", FieldWriter::writeGrid(map.paths));
	}
	const bool hasFront =
		std::any_of(map.front.begin(), map.front.end(),
			    [](const std::optional<TileRef>& tile) {
				    return tile.has_value();
			    });
	if (hasFront) {
		table.insert("front", FieldWriter::writeGrid(map.front));
	}
	if (!map.objects.empty()) {
		table.insert("objects", writeObjects(map));
	}
	if (!map.markers.empty()) {
		table.insert("markers", writeMarkers(map));
	}
	if (!map.portals.empty()) {
		table.insert("portals", writePortals(map));
	}
	return table;
}

toml::array WorldTomlWriter::writeObjects(const Map& map) const {
	toml::array array;
	for (const MapObject& object : map.objects) {
		toml::table entry;
		entry.insert("role", vocabulary_.toText(object.role));
		entry.insert("tile", TileCodec::encode(object.tile));
		entry.insert("cell",
			     FieldWriter::writePair(object.cell.column,
							object.cell.row));
		entry.insert("size",
			     FieldWriter::writePair(object.width,
							object.height));
		entry.insert("blocked", object.blocked);
		if (object.door.has_value()) {
			entry.insert("door",
				     FieldWriter::writePair(
					     object.door->column,
					     object.door->row));
			if (object.doorWidth > 1) {
				entry.insert("door_width", object.doorWidth);
			}
		}
		if (!object.collisionGrid.empty()) {
			entry.insert("collision_grid",
				     FieldWriter::writeBooleans(
					     object.collisionGrid));
		}
		array.push_back(std::move(entry));
	}
	return array;
}

toml::array WorldTomlWriter::writeMarkers(const Map& map) const {
	toml::array array;
	for (const Marker& marker : map.markers) {
		toml::table entry;
		entry.insert("kind", vocabulary_.toText(marker.kind));
		entry.insert("cell",
			     FieldWriter::writePair(marker.cell.column,
							marker.cell.row));
		array.push_back(std::move(entry));
	}
	return array;
}

toml::array WorldTomlWriter::writePortals(const Map& map) const {
	toml::array array;
	for (const Portal& portal : map.portals) {
		toml::table entry;
		entry.insert("kind", vocabulary_.toText(portal.kind));
		entry.insert("cell",
			     FieldWriter::writePair(portal.cell.column,
							portal.cell.row));
		entry.insert("target_map", portal.targetMap);
		entry.insert("target_cell",
			     FieldWriter::writePair(
				     portal.targetCell.column,
				     portal.targetCell.row));
		if (portal.width != 1 || portal.height != 1) {
			entry.insert("size",
				     FieldWriter::writePair(portal.width,
							    portal.height));
		}
		array.push_back(std::move(entry));
	}
	return array;
}

}
