#include "world/SpriteVisual.h"

#include <algorithm>
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

	// Celdas que abarca: opcional (default 1x1).
	const std::string spanPath = base + ".span";
	if (doc.hasKey(spanPath)) {
		const std::array<int, 2> span = doc.getIntArray<2>(spanPath);
		if (span[0] <= 0 || span[1] <= 0) {
			throw ConfigError(ConfigErrorKind::InvalidValue,
					  spanPath);
		}
		cols = span[0];
		rows = span[1];
	}

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
	return drawnWidth > 0 ? drawnWidth : cellWidth * cols;
}

int SpriteVisual::displayHeight() const {
	return drawnHeight > 0 ? drawnHeight : cellHeight * rows;
}

PixelBox SpriteVisual::fitIn(const PixelBox& box) const {
	const int dispW = displayWidth();
	const int dispH = displayHeight();
	if (dispW <= 0 || dispH <= 0) {
		return box;
	}
	const double scale = std::min(static_cast<double>(box.width) / dispW,
				      static_cast<double>(box.height) / dispH);
	const int width = std::max(1, static_cast<int>(dispW * scale));
	const int height = std::max(1, static_cast<int>(dispH * scale));
	return PixelBox{box.x + (box.width - width) / 2,
			box.y + (box.height - height) / 2, width, height};
}

SheetRegion::SheetRegion(const SheetCell& topLeft, int colsValue,
			 int rowsValue)
	: at(topLeft), cols(colsValue), rows(rowsValue) {}

}
