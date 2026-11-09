#pragma once

#include "world/ScenePixelMapper.h"

namespace sdl {

class TileLayout {
public:
	struct Placement {
		int offsetX;
		int offsetY;
		int width;
		int height;
	};

	struct Range {
		int firstColumn;
		int lastColumn;
		int firstRow;
		int lastRow;

		bool empty() const;
	};

	TileLayout(const world::PixelRect& area, int pieceWidth,
		   int pieceHeight);

	Range visibleTiled(const world::PixelRect& clip) const;
	Range visibleStrip(const world::PixelRect& clip) const;

	Placement tiledAt(int column, int row) const;
	Placement stripAt(int column) const;

private:
	bool drawable() const;
	int firstIndex(int origin, int piece, int clipOrigin) const;
	int lastIndex(int origin, int size, int piece, int clipEnd) const;

	const world::PixelRect area_;
	const int pieceWidth_;
	const int pieceHeight_;
};

}  // namespace sdl
