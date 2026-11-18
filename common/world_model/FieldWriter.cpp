#include "world_model/FieldWriter.h"

#include <optional>
#include <utility>

#include "world_model/TileCodec.h"

namespace worldmodel {

toml::array FieldWriter::writePair(int first, int second) {
	return toml::array{first, second};
}

toml::array FieldWriter::writeBooleans(
	const std::vector<bool>& values) {
	toml::array array;
	for (const bool value : values) {
		array.push_back(value);
	}
	return array;
}

toml::array FieldWriter::writeGrid(const TileGrid& grid) {
	toml::array array;
	for (const std::optional<TileRef>& tile : grid) {
		array.push_back(TileCodec::encode(tile));
	}
	return array;
}

toml::table FieldWriter::writeTilesets(
	const std::map<std::string, TilesetDef>& tilesets) {
	toml::table table;
	for (const auto& [name, tileset] : tilesets) {
		toml::table entry;
		entry.insert("image", tileset.image);
		entry.insert("cell",
			     writePair(tileset.cellWidth, tileset.cellHeight));
		table.insert(name, std::move(entry));
	}
	return table;
}

}
