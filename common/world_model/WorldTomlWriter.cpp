#include "world_model/WorldTomlWriter.h"

#include <algorithm>
#include <optional>
#include <sstream>
#include <utility>
#include <vector>

#include <toml++/toml.hpp>

#include "world_model/TileCodec.h"

namespace worldmodel {

toml::array WorldTomlWriter::writePair(int first, int second) const {
	return toml::array{first, second};
}

toml::array WorldTomlWriter::writeBooleans(
	const std::vector<bool>& values) const {
	toml::array array;
	for (const bool value : values) {
		array.push_back(value);
	}
	return array;
}

toml::array WorldTomlWriter::writeGrid(const TileGrid& grid) const {
	toml::array array;
	for (const std::optional<TileRef>& tile : grid) {
		array.push_back(TileCodec::encode(tile));
	}
	return array;
}

toml::array WorldTomlWriter::writeObjects(const Map& map) const {
	toml::array array;
	for (const MapObject& object : map.objects) {
		toml::table entry;
		entry.insert("role", vocabulary_.toText(object.role));
		entry.insert("tile", TileCodec::encode(object.tile));
		entry.insert("cell",
			     writePair(object.cell.column, object.cell.row));
		entry.insert("size", writePair(object.width, object.height));
		entry.insert("blocked", object.blocked);
		if (object.door.has_value()) {
			entry.insert(
				"door",
				writePair(object.door->column, object.door->row));
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
			     writePair(marker.cell.column, marker.cell.row));
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
			     writePair(portal.cell.column, portal.cell.row));
		entry.insert("target_map", portal.targetMap);
		entry.insert(
			"target_cell",
			writePair(portal.targetCell.column, portal.targetCell.row));
		array.push_back(std::move(entry));
	}
	return array;
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
	table.insert("base_collision", writeBooleans(map.baseCollision));
	table.insert("collision", writeBooleans(map.collision));
	table.insert("ground", writeGrid(map.ground));

	const bool hasPaths =
		std::any_of(map.paths.begin(), map.paths.end(),
			    [](const std::optional<TileRef>& tile) {
				    return tile.has_value();
			    });
	if (hasPaths) {
		table.insert("paths", writeGrid(map.paths));
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

toml::table WorldTomlWriter::writeTilesets(const World& world) const {
	toml::table table;
	for (const auto& [name, tileset] : world.tilesets) {
		toml::table entry;
		entry.insert("image", tileset.image);
		entry.insert("cell",
			     writePair(tileset.cellWidth, tileset.cellHeight));
		table.insert(name, std::move(entry));
	}
	return table;
}

toml::table WorldTomlWriter::writeWorld(const World& world) const {
	toml::table table;
	table.insert("slug", world.slug);
	table.insert("name", world.name);
	if (!world.description.empty()) {
		table.insert("description", world.description);
	}
	return table;
}

std::string WorldTomlWriter::write(const World& world) const {
	toml::table table;
	table.insert("format_version", world.formatVersion);
	table.insert("world", writeWorld(world));
	table.insert("tilesets", writeTilesets(world));

	toml::table maps;
	for (const auto& [id, map] : world.maps) {
		maps.insert(id, writeMap(map));
	}
	table.insert("maps", std::move(maps));

	std::ostringstream stream;
	stream << table;
	return stream.str();
}

}
