#include "TilesetCache.h"

#include <QPainter>
#include <QString>
#include <algorithm>
#include <utility>

#include "Constants.h"

TilesetCache::TilesetCache(std::string assetsDir)
	: assetsDir_(std::move(assetsDir)) {}

void TilesetCache::loadTilesets(
		const std::map<std::string, worldmodel::TilesetDef>& tilesets) {
	for (const auto& [id, def] : tilesets) {
		auto it = entries_.find(id);
		if (it != entries_.end() && it->second.def == def) {
			continue;
		}
		QString path = QString::fromStdString(assetsDir_ + "/" + def.image);
		entries_[id] = Entry{def, QPixmap(path)};
	}
}

QPixmap TilesetCache::tilePixmap(const worldmodel::TileRef& ref, int widthCells,
				 int heightCells) const {
	auto it = entries_.find(ref.tileset);
	int cellW = editor::kDefaultCellSize;
	int cellH = editor::kDefaultCellSize;
	if (it != entries_.end() && it->second.def.cellWidth > 0 &&
	    it->second.def.cellHeight > 0) {
		cellW = it->second.def.cellWidth;
		cellH = it->second.def.cellHeight;
	}

	if (widthCells <= 0 || heightCells <= 0) {
		return makeFallback(std::max(1, widthCells) * cellW,
				    std::max(1, heightCells) * cellH);
	}

	const int widthPx = widthCells * cellW;
	const int heightPx = heightCells * cellH;
	if (it == entries_.end() || it->second.sheet.isNull()) {
		return makeFallback(widthPx, heightPx);
	}

	const QPixmap& sheet = it->second.sheet;
	const int x = ref.column * cellW;
	const int y = ref.row * cellH;
	if (x < 0 || y < 0 || x + widthPx > sheet.width() ||
	    y + heightPx > sheet.height()) {
		return makeFallback(widthPx, heightPx);
	}

	return sheet.copy(x, y, widthPx, heightPx);
}

QPixmap TilesetCache::makeFallback(int width, int height) {
	QPixmap pixmap(width, height);
	pixmap.fill(Qt::black);
	QPainter painter(&pixmap);
	const QColor magenta(255, 0, 255);
	for (int y = 0; y < height; y += editor::kFallbackSquareSize) {
		for (int x = 0; x < width; x += editor::kFallbackSquareSize) {
			bool even = ((x / editor::kFallbackSquareSize) +
				     (y / editor::kFallbackSquareSize)) %
					    2 ==
				    0;
			if (even) {
				painter.fillRect(x, y,
						 editor::kFallbackSquareSize,
						 editor::kFallbackSquareSize,
						 magenta);
			}
		}
	}
	return pixmap;
}
