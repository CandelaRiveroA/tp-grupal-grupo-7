#pragma once

#include <optional>
#include <string>

#include "world_model/Tile.h"

namespace worldmodel {

class TileCodec {
public:
	// "" -> nullopt; texto invalido -> FormatError(InvalidValue, path).
	static std::optional<TileRef> decode(const std::string& text,
					     const std::string& path);
	// nullopt -> ""; tile -> "tileset:column,row".
	static std::string encode(const std::optional<TileRef>& tile);

private:
	static bool parseInt(const std::string& text, int& value);
};

}
