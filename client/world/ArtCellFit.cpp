#include "world/ArtCellFit.h"

#include "world/ConfigError.h"
#include "world/SceneKeys.h"

namespace world {

int ArtCellFit::scaleFor(const TileSetDef& tileset, int cellPixelSize,
			 const std::string& tilesetName) const {
	const bool exact = tileset.cellWidth > 0 && tileset.cellHeight > 0 &&
			   cellPixelSize % tileset.cellWidth == 0 &&
			   cellPixelSize % tileset.cellHeight == 0 &&
			   cellPixelSize / tileset.cellWidth ==
				   cellPixelSize / tileset.cellHeight;
	if (!exact) {
		throw ConfigError(ConfigErrorKind::InvalidValue,
				  std::string(kKeyTilesets) + "." + tilesetName +
					  "." + kKeyCell);
	}
	return cellPixelSize / tileset.cellWidth;
}

}
