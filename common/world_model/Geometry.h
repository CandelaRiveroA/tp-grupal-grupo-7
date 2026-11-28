#pragma once

namespace worldmodel {

struct TilePos {
	int column = 0;
	int row = 0;
};

// Cercanía de Chebyshev a la huella rectangular, incluido su interior.
inline bool withinInteractionRange(TilePos origin, int width, int height, TilePos point, int ring) {
    return point.column >= origin.column - ring &&
           point.column <= origin.column + width - 1 + ring &&
           point.row >= origin.row - ring &&
           point.row <= origin.row + height - 1 + ring;
}

bool operator==(const TilePos& left, const TilePos& right);
bool operator<(const TilePos& left, const TilePos& right);

}
