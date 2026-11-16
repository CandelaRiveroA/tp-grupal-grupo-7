#include "world_model/FieldReader.h"

#include <array>
#include <optional>
#include <utility>
#include <vector>

#include "world_model/FormatError.h"
#include "world_model/TileCodec.h"

namespace worldmodel {

FieldReader::FieldReader(const tomlcfg::TomlDocument& doc)
	: doc_(doc) {}

int FieldReader::readFormatVersion() const {
	const int version = doc_.getInt("format_version");
	if (version != 1) {
		throw FormatError(FormatErrorKind::InvalidValue,
				  "format_version");
	}
	return version;
}

std::string FieldReader::join(const std::string& base,
				  const std::string& key) const {
	return base + "." + key;
}

std::string FieldReader::idx(const std::string& base,
				 std::size_t index) const {
	return base + "[" + std::to_string(index) + "]";
}

int FieldReader::readPositiveInt(const std::string& path) const {
	const int value = doc_.getInt(path);
	if (value <= 0) {
		throw FormatError(FormatErrorKind::InvalidValue, path);
	}
	return value;
}

TilePos FieldReader::readTilePos(const std::string& path) const {
	const std::array<int, 2> values = doc_.getIntArray<2>(path);
	return TilePos{values[0], values[1]};
}

std::map<std::string, TilesetDef> FieldReader::readTilesets(
	const std::string& path) const {
	std::map<std::string, TilesetDef> tilesets;
	for (const std::string& name : doc_.subtableKeys(path)) {
		const std::string tilesetPath = join(path, name);
		const std::string cellPath = join(tilesetPath, "cell");
		const std::array<int, 2> cell =
			doc_.getIntArray<2>(cellPath);
		if (cell[0] <= 0 || cell[1] <= 0) {
			throw FormatError(FormatErrorKind::InvalidValue, cellPath);
		}
		tilesets.emplace(
			name,
			TilesetDef{doc_.getString(join(tilesetPath, "image")),
				   cell[0], cell[1]});
	}
	return tilesets;
}

TileGrid FieldReader::readGrid(
	const std::string& path, int width, int height,
	const std::map<std::string, TilesetDef>& tilesets) const {
	const std::vector<std::string> texts = doc_.getStringArray(path);
	const std::size_t cells = static_cast<std::size_t>(width) *
				  static_cast<std::size_t>(height);
	if (texts.size() != cells) {
		throw FormatError(FormatErrorKind::InvalidValue, path);
	}

	TileGrid grid;
	grid.reserve(cells);
	for (std::size_t i = 0; i < texts.size(); ++i) {
		const std::string cellPath = idx(path, i);
		const std::optional<TileRef> tile =
			TileCodec::decode(texts[i], cellPath);
		if (tile.has_value()) {
			checkTileset(tilesets, *tile, cellPath);
		}
		grid.push_back(tile);
	}
	return grid;
}

TileRef FieldReader::readTileRef(
	const std::string& path,
	const std::map<std::string, TilesetDef>& tilesets) const {
	const std::optional<TileRef> tile =
		TileCodec::decode(doc_.getString(path), path);
	if (!tile.has_value()) {
		throw FormatError(FormatErrorKind::InvalidValue, path);
	}
	checkTileset(tilesets, *tile, path);
	return *tile;
}

void FieldReader::checkTileset(
	const std::map<std::string, TilesetDef>& tilesets,
	const TileRef& tile, const std::string& path) const {
	if (tilesets.find(tile.tileset) == tilesets.end()) {
		throw FormatError(FormatErrorKind::InvalidValue, path);
	}
}

}
