#include "world_model/FieldWriter.h"

#include <algorithm>
#include <cctype>
#include <cstddef>
#include <optional>
#include <ostream>
#include <sstream>
#include <utility>

#include "world_model/TileCodec.h"

namespace worldmodel {

toml::array FieldWriter::writePair(int first, int second) const {
	return toml::array{first, second};
}

toml::table FieldWriter::writeTilesets(
	const std::map<std::string, TilesetDef>& tilesets) const {
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

std::string FieldWriter::formatKey(std::string_view key) const {
	if (isBareKey(key)) {
		return std::string(key);
	}
	std::ostringstream stream;
	stream << toml::value<std::string>(std::string(key));
	return stream.str();
}

void FieldWriter::appendBoolGrid(std::ostream& out, std::string_view key,
				 const std::vector<bool>& values,
				 int columns) const {
	std::vector<std::string> items;
	items.reserve(values.size());
	for (const bool value : values) {
		items.emplace_back(value ? "true" : "false");
	}
	appendGrid(out, key, items, columns);
}

void FieldWriter::appendTileGrid(std::ostream& out, std::string_view key,
				 const TileGrid& grid, int columns) const {
	std::vector<std::string> items;
	items.reserve(grid.size());
	for (const std::optional<TileRef>& tile : grid) {
		std::ostringstream item;
		item << toml::value<std::string>(TileCodec::encode(tile));
		items.push_back(item.str());
	}
	appendGrid(out, key, items, columns);
}

void FieldWriter::appendOptionalTileGrid(std::ostream& out,
					 std::string_view key,
					 const TileGrid& grid,
					 int columns) const {
	const bool hasAny = std::any_of(
		grid.begin(), grid.end(),
		[](const std::optional<TileRef>& tile) {
			return tile.has_value();
		});
	if (hasAny) {
		appendTileGrid(out, key, grid, columns);
	}
}

bool FieldWriter::isBareKey(std::string_view key) const {
	if (key.empty()) {
		return false;
	}
	return std::all_of(key.begin(), key.end(), [](unsigned char ch) {
		return std::isalnum(ch) != 0 || ch == '_' || ch == '-';
	});
}

void FieldWriter::appendGrid(std::ostream& out, std::string_view key,
			     const std::vector<std::string>& items,
			     int columns) const {
	const std::size_t width = columns > 0
					  ? static_cast<std::size_t>(columns)
					  : std::max<std::size_t>(items.size(), 1);
	// Una fila del mapa por línea; TOML acepta la coma final del array.
	out << key << " = [\n";
	for (std::size_t row = 0; row < items.size(); row += width) {
		const std::size_t end = std::min(row + width, items.size());
		out << "    ";
		for (std::size_t i = row; i < end; ++i) {
			out << items[i] << (i + 1 < end ? ", " : ",\n");
		}
	}
	out << "]\n";
}

}
