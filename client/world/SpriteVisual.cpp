#include "world/SpriteVisual.h"

#include <array>

#include "toml/TomlDocument.h"
#include "world/ConfigError.h"

namespace world {

SheetCell::SheetCell(int colValue, int rowValue)
	: col(colValue), row(rowValue) {}

SheetCell::SheetCell(const tomlcfg::TomlDocument& doc, const std::string& key) {
	const std::array<int, 2> pair = doc.getIntArray<2>(key);
	if (pair[0] < 0 || pair[1] < 0) {
		throw ConfigError(ConfigErrorKind::InvalidValue, key);
	}
	col = pair[0];
	row = pair[1];
}

SpriteVisual::SpriteVisual(const tomlcfg::TomlDocument& doc,
			   const std::string& base) {
	const std::string sheetPath = base + ".sheet";
	sheetImage = doc.getString(sheetPath);
	if (sheetImage.empty()) {
		throw ConfigError(ConfigErrorKind::InvalidValue, sheetPath);
	}

	const std::string cellPath = base + ".cell";
	const std::array<int, 2> cell = doc.getIntArray<2>(cellPath);
	if (cell[0] <= 0 || cell[1] <= 0) {
		throw ConfigError(ConfigErrorKind::InvalidValue, cellPath);
	}
	cellWidth = cell[0];
	cellHeight = cell[1];

	// Tamaño EN EL MAPA: opcional (default = celda nativa).
	const std::string drawnPath = base + ".drawn";
	if (doc.hasKey(drawnPath)) {
		const std::array<int, 2> drawn = doc.getIntArray<2>(drawnPath);
		if (drawn[0] <= 0 || drawn[1] <= 0) {
			throw ConfigError(ConfigErrorKind::InvalidValue,
					  drawnPath);
		}
		drawnWidth = drawn[0];
		drawnHeight = drawn[1];
	}

	// Ajuste vertical al apoyarlo en el piso: opcional (default 0 =
	// pegado a los pies). Compensa el aire transparente del sprite.
	const std::string offsetPath = base + ".offset_y";
	if (doc.hasKey(offsetPath)) {
		offsetY = doc.getInt(offsetPath);
	}
}

SpriteVisual::SpriteVisual(const tomlcfg::TomlDocument& doc,
			   const std::string& base, const std::string& posKey)
	: SpriteVisual(doc, base) {
	const SheetCell position(doc, base + "." + posKey);
	col = position.col;
	row = position.row;
}

int SpriteVisual::displayWidth() const {
	return drawnWidth > 0 ? drawnWidth : cellWidth;
}

int SpriteVisual::displayHeight() const {
	return drawnHeight > 0 ? drawnHeight : cellHeight;
}

}
