#pragma once

#include <vector>

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

	TileLayout(const world::PixelRect& area, int pieceWidth,
		   int pieceHeight);

	std::vector<Placement> tiled() const;

	std::vector<Placement> strip() const;

private:
	bool drawable() const;

	const world::PixelRect area_;
	const int pieceWidth_;
	const int pieceHeight_;
};

}
