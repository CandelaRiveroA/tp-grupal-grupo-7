#include "sdl/render/TileLayout.h"

#include <algorithm>

namespace sdl {

TileLayout::TileLayout(const world::PixelRect& area, int pieceWidth,
		       int pieceHeight)
	: area_(area)
	, pieceWidth_(pieceWidth)
	, pieceHeight_(pieceHeight) {}

std::vector<TileLayout::Placement> TileLayout::tiled() const {
	std::vector<Placement> placements;
	if (!drawable()) {
		return placements;
	}
	for (int offsetY = 0; offsetY < area_.height;
	     offsetY += pieceHeight_) {
		const int cellHeight =
			std::min(pieceHeight_, area_.height - offsetY);
		for (int offsetX = 0; offsetX < area_.width;
		     offsetX += pieceWidth_) {
			const int cellWidth =
				std::min(pieceWidth_, area_.width - offsetX);
			placements.push_back(
				{offsetX, offsetY, cellWidth, cellHeight});
		}
	}
	return placements;
}

std::vector<TileLayout::Placement> TileLayout::strip() const {
	std::vector<Placement> placements;
	if (!drawable()) {
		return placements;
	}
	for (int offsetX = 0; offsetX < area_.width;
	     offsetX += pieceWidth_) {
		const int cellWidth =
			std::min(pieceWidth_, area_.width - offsetX);
		placements.push_back(
			{offsetX, 0, cellWidth, pieceHeight_});
	}
	return placements;
}

bool TileLayout::drawable() const {
	return area_.width > 0 && area_.height > 0 && pieceWidth_ > 0 &&
	       pieceHeight_ > 0;
}

}
