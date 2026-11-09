#include "sdl/render/TileLayout.h"

#include <algorithm>

namespace sdl {

TileLayout::TileLayout(const world::PixelRect& area, int pieceWidth,
		       int pieceHeight)
	: area_(area)
	, pieceWidth_(pieceWidth)
	, pieceHeight_(pieceHeight) {}

bool TileLayout::Range::empty() const {
	return firstColumn > lastColumn || firstRow > lastRow;
}

TileLayout::Range TileLayout::visibleTiled(
	const world::PixelRect& clip) const {
	if (!drawable() || clip.width <= 0 || clip.height <= 0) {
		return Range{0, -1, 0, -1};
	}

	const int firstColumn = firstIndex(area_.x, pieceWidth_, clip.x);
	const int lastColumn = lastIndex(area_.x, area_.width, pieceWidth_,
					  clip.x + clip.width);
	const int firstRow = firstIndex(area_.y, pieceHeight_, clip.y);
	const int lastRow = lastIndex(area_.y, area_.height, pieceHeight_,
				      clip.y + clip.height);
	return Range{firstColumn, lastColumn, firstRow, lastRow};
}

TileLayout::Range TileLayout::visibleStrip(
	const world::PixelRect& clip) const {
	if (!drawable() || clip.width <= 0 || clip.height <= 0) {
		return Range{0, -1, 0, -1};
	}

	const int firstColumn = firstIndex(area_.x, pieceWidth_, clip.x);
	const int lastColumn = lastIndex(area_.x, area_.width, pieceWidth_,
					  clip.x + clip.width);
	const bool rowVisible = clip.y < area_.y + pieceHeight_ &&
				clip.y + clip.height > area_.y;
	return Range{firstColumn, lastColumn, 0, rowVisible ? 0 : -1};
}

TileLayout::Placement TileLayout::tiledAt(const int column,
					  const int row) const {
	const int offsetX = column * pieceWidth_;
	const int offsetY = row * pieceHeight_;
	return Placement{offsetX, offsetY,
			 std::min(pieceWidth_, area_.width - offsetX),
			 std::min(pieceHeight_, area_.height - offsetY)};
}

TileLayout::Placement TileLayout::stripAt(const int column) const {
	const int offsetX = column * pieceWidth_;
	return Placement{offsetX, 0,
			 std::min(pieceWidth_, area_.width - offsetX),
			 pieceHeight_};
}

bool TileLayout::drawable() const {
	return area_.width > 0 && area_.height > 0 && pieceWidth_ > 0 &&
	       pieceHeight_ > 0;
}

int TileLayout::firstIndex(const int origin, const int piece,
			   const int clipOrigin) const {
	const int relative = clipOrigin - origin;
	return relative > 0 ? relative / piece : 0;
}

int TileLayout::lastIndex(const int origin, const int size, const int piece,
			  const int clipEnd) const {
	if (clipEnd <= origin) {
		return -1;
	}
	const int relative = clipEnd - origin;
	const int visibleCount = (relative + piece - 1) / piece;
	const int totalCount = (size + piece - 1) / piece;
	return std::min(totalCount, visibleCount) - 1;
}

}  // namespace sdl
