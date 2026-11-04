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

PixelRect ScenePixelMapper::worldBounds(const GridSpan& grid) const {
	return toPixels(GridCell{kOriginColumn, kOriginRow}, grid);
}

}
