#include "world/ScenePixelMapper.h"

#include "world/WorldConstants.h"

namespace world {

ScenePixelMapper::ScenePixelMapper(int cellPixelSize)
	: cellPixelSize_(cellPixelSize) {}

int ScenePixelMapper::cellPixelSize() const { return cellPixelSize_; }

PixelRect ScenePixelMapper::toPixels(const GridCell& at,
				     const GridSpan& span) const {
	return PixelRect{at.column * cellPixelSize_, at.row * cellPixelSize_,
			 span.columns * cellPixelSize_,
			 span.rows * cellPixelSize_};
}

int ScenePixelMapper::cellIndex(const int pixels) const {
	int index = pixels / cellPixelSize_;
	if (pixels < 0 && pixels % cellPixelSize_ != 0) {
		--index;  // división hacia -infinito (la truncada da 0)
	}
	return index;
}

GridCell ScenePixelMapper::toCell(const PixelPoint& point) const {
	return GridCell{cellIndex(point.x), cellIndex(point.y)};
}

PixelRect ScenePixelMapper::worldBounds(const GridSpan& grid) const {
	return toPixels(GridCell{kOriginColumn, kOriginRow}, grid);
}

}
