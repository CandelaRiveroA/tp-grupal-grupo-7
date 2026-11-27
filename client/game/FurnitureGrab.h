#pragma once

#include <string>

#include "world/Grid.h"

namespace game {

struct FurnitureGrab {
	std::string contentId;
	world::GridCell cell;
};

}
