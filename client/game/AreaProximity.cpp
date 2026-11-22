#include "game/AreaProximity.h"

namespace game {

AreaProximity::AreaProximity(int ring) : ring_(ring) {}

bool AreaProximity::around(const world::GridCell& at,
			   const world::GridSpan& span, int cellX,
			   int cellY) const {
	const int left = at.column - ring_;
	const int top = at.row - ring_;
	const int right = at.column + span.columns - 1 + ring_;
	const int bottom = at.row + span.rows - 1 + ring_;
	return cellX >= left && cellX <= right && cellY >= top &&
	       cellY <= bottom;
}

}
