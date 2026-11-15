#pragma once

#include <string>

#include "world_model/Geometry.h"

namespace worldmodel {

enum class PortalKind {
	TownEntrance,
	FarmEntrance,
	DungeonEntrance,
	StairsDown,
	DungeonExit,
	InteriorExit,
	Link,   // portal libre entre mapas; destino arbitrario
};

struct Portal {
	PortalKind kind = PortalKind::TownEntrance;
	TilePos cell;
	std::string targetMap;
	TilePos targetCell;
};

}
