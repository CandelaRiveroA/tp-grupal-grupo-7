#pragma once

#include "world/FurnitureFacing.h"
#include "world/Grid.h"

namespace game {

// Intención de colocar el mueble del slot `slot` en `cell`: la celda del
// mueble que entiende la fuente (la misma de ViewFurniture). Es lo que el jugador PIDE; la fuente de estado
// decide si se puede.
struct FurniturePlacement {
	world::GridCell cell;
	int slot;
	world::FurnitureFacing facing = world::FurnitureFacing::Front;
};

}
