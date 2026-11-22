#include "world/TileLayerLayout.h"

#include <algorithm>

namespace world {

bool TileLayerLayout::Range::empty() const {
	return firstColumn > lastColumn || firstRow > lastRow;
}

TileLayerLayout::TileLayerLayout(const ScenePixelMapper& mapper)
	: mapper_(mapper) {}

TileLayerLayout::Range TileLayerLayout::visible(
	const GridSpan& size, const PixelRect& visibleWorld) const {
	const Range none{0, -1, 0, -1};
	if (size.columns <= 0 || size.rows <= 0 || visibleWorld.width <= 0 ||
	    visibleWorld.height <= 0) {
		return none;
	}
	const GridCell first =
		mapper_.toCell(PixelPoint{visibleWorld.x, visibleWorld.y});
	const GridCell last = mapper_.toCell(
		PixelPoint{visibleWorld.x + visibleWorld.width - 1,
			   visibleWorld.y + visibleWorld.height - 1});
	const Range range{std::max(first.column, 0),
			  std::min(last.column, size.columns - 1),
			  std::max(first.row, 0),
			  std::min(last.row, size.rows - 1)};
	return range.empty() ? none : range;
}

PixelRect TileLayerLayout::cellRect(int column, int row) const {
	return mapper_.toPixels(GridCell{column, row}, GridSpan{1, 1});
}

}
