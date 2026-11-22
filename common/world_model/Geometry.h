#pragma once

namespace worldmodel {

struct TilePos {
	int column = 0;
	int row = 0;
};

bool operator==(const TilePos& left, const TilePos& right);
bool operator<(const TilePos& left, const TilePos& right);

}
