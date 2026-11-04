#include "world/Grid.h"

namespace world {

bool GridCell::operator<(const GridCell& other) const {
	if (row != other.row) {
		return row < other.row;
	}
	return column < other.column;
}

}
