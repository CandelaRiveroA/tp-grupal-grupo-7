#pragma once

namespace worldmodel {

struct TilePos {
	int column = 0;
	int row = 0;
};

// Cercanía de Chebyshev a la huella rectangular, incluido su interior.
bool withinInteractionRange(TilePos origin, int width, int height, TilePos point, int ring);

bool operator==(const TilePos& left, const TilePos& right);
bool operator<(const TilePos& left, const TilePos& right);

}
