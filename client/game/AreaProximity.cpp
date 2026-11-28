#include "game/AreaProximity.h"

#include "world_model/Geometry.h"

namespace game {

AreaProximity::AreaProximity(int ring) : ring_(ring) {}

bool AreaProximity::around(const world::GridCell& at,
			   const world::GridSpan& span, int cellX,
			   int cellY) const {
	return worldmodel::withinInteractionRange(
        {at.column, at.row}, span.columns, span.rows, {cellX, cellY}, ring_);
}

}
