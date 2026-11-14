#pragma once

#include <cstddef>
#include <map>
#include <string>

#include "toml/TomlDocument.h"
#include "world_model/Geometry.h"
#include "world_model/Tile.h"

namespace worldmodel {

// Lectura de campos TOML con las validaciones comunes a todos los formatos
// del modelo. No conoce World ni Scenario: recibe y devuelve solo datos.
class FieldReader {
public:
	explicit FieldReader(const tomlcfg::TomlDocument& doc);

	int readFormatVersion() const;
	std::string join(const std::string& base, const std::string& key) const;
	std::string idx(const std::string& base, std::size_t index) const;
	int readPositiveInt(const std::string& path) const;
	TilePos readTilePos(const std::string& path) const;
	std::map<std::string, TilesetDef> readTilesets(
		const std::string& path) const;
	TileGrid readGrid(const std::string& path, int width, int height,
			  const std::map<std::string, TilesetDef>& tilesets) const;
	void checkTileset(const std::map<std::string, TilesetDef>& tilesets,
			  const TileRef& tile, const std::string& path) const;

private:
	const tomlcfg::TomlDocument& doc_;
};

}
