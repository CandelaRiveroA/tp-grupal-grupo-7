#include "world_model/WorldTomlWriter.h"

#include <sstream>
#include <utility>

#include <toml++/toml.hpp>

#include "world_model/TileCodec.h"

namespace worldmodel {

std::string WorldTomlWriter::write(const World& world) const {
	toml::table worldTable;
	worldTable.insert("name", world.name);

	toml::table header;
	header.insert("format_version", world.formatVersion);
	header.insert("world", std::move(worldTable));
	header.insert("tilesets", fields_.writeTilesets(world.tilesets));

	std::ostringstream stream;
	stream << header << '\n';
	if (world.maps.empty()) {
		stream << "\n[maps]\n";
		return stream.str();
	}
	for (const auto& [id, map] : world.maps) {
		writeMap(stream, id, map);
	}
	return stream.str();
}

void WorldTomlWriter::writeMap(std::ostream& out, const std::string& id,
			       const Map& map) const {
	const std::string mapKey = fields_.formatKey(id);
	out << "\n[maps." << mapKey << "]\n";

	toml::table scalars;
	scalars.insert("kind", vocabulary_.toText(map.kind));
	if (map.kind == MapKind::Dungeon) {
		scalars.insert("level", map.level);
	}
	scalars.insert("width", map.width);
	scalars.insert("height", map.height);
	scalars.insert("tile_size", map.tileSize);
	out << scalars << '\n';

	fields_.appendBoolGrid(out, "base_collision", map.baseCollision,
			       map.width);
	fields_.appendBoolGrid(out, "collision", map.collision, map.width);
	fields_.appendTileGrid(out, "ground", map.ground, map.width);
	fields_.appendOptionalTileGrid(out, "paths", map.paths, map.width);
	fields_.appendOptionalTileGrid(out, "front", map.front, map.width);

	writeObjects(out, mapKey, map);
	writeMarkers(out, mapKey, map);
	writePortals(out, mapKey, map);
}

void WorldTomlWriter::writeObjects(std::ostream& out, const std::string& mapKey,
				   const Map& map) const {
	for (const MapObject& object : map.objects) {
		out << "\n[[maps." << mapKey << ".objects]]\n";
		toml::table entry;
		entry.insert("role", vocabulary_.toText(object.role));
		entry.insert("tile", TileCodec::encode(object.tile));
		entry.insert("cell", fields_.writePair(object.cell.column,
						       object.cell.row));
		entry.insert("size",
			     fields_.writePair(object.width, object.height));
		entry.insert("blocked", object.blocked);
		if (object.door.has_value()) {
			entry.insert("door",
				     fields_.writePair(object.door->column,
						       object.door->row));
			if (object.doorWidth > 1) {
				entry.insert("door_width", object.doorWidth);
			}
		}
		out << entry << '\n';
		if (!object.collisionGrid.empty()) {
			fields_.appendBoolGrid(out, "collision_grid",
					       object.collisionGrid,
					       object.width);
		}
	}
}

void WorldTomlWriter::writeMarkers(std::ostream& out, const std::string& mapKey,
				   const Map& map) const {
	for (const Marker& marker : map.markers) {
		out << "\n[[maps." << mapKey << ".markers]]\n";
		toml::table entry;
		entry.insert("kind", vocabulary_.toText(marker.kind));
		entry.insert("cell", fields_.writePair(marker.cell.column,
						       marker.cell.row));
		out << entry << '\n';
	}
}

void WorldTomlWriter::writePortals(std::ostream& out, const std::string& mapKey,
				   const Map& map) const {
	for (const Portal& portal : map.portals) {
		out << "\n[[maps." << mapKey << ".portals]]\n";
		toml::table entry;
		entry.insert("kind", vocabulary_.toText(portal.kind));
		entry.insert("cell", fields_.writePair(portal.cell.column,
						       portal.cell.row));
		entry.insert("target_map", portal.targetMap);
		entry.insert("target_cell",
			     fields_.writePair(portal.targetCell.column,
					       portal.targetCell.row));
		if (portal.width != 1 || portal.height != 1) {
			entry.insert("size", fields_.writePair(portal.width,
							       portal.height));
		}
		out << entry << '\n';
	}
}

}
