#pragma once

#include <optional>
#include <vector>

#include "common/common_game_direction.h"
#include "world/CreatureCatalog.h"
#include "world/Grid.h"
#include "world/ItemCatalog.h"
#include "world/ResourceCatalog.h"

namespace game {


struct ViewEntity {
	world::CreatureKind kind;
	int cellX;
	int cellY;
	protocol::Direction facing;
	bool moving;
	world::ActionKind action = world::ActionKind::Idle;
	// Herramienta activa durante UseTool (la decide la fuente). nullopt
	// si no usa herramienta.
	std::optional<world::ItemKind> activeTool;
};

// Recurso picable. La fuente decide tipo, celda y estado; al agotarse
// sale del vector resources.
struct ViewResource {
	world::ResourceKind kind;
	int cellX;
	int cellY;
	world::ResourceState state;
};

struct ViewInventorySlot {
	int slotIndex;
	world::ItemKind item;
};

// Evento puntual del tick: la fuente informa que el jugador OBTUVO un
// recurso (no lo deduce el cliente). Se limpia cada tick.
struct ViewResourceObtained {
	world::ResourceKind kind;
};

struct LocalWorldView {
	std::vector<ViewEntity> entities;
	std::vector<ViewResource> resources;
	std::vector<ViewResourceObtained> obtained;
	std::vector<ViewInventorySlot> inventory;
	world::GridSpan grid{0, 0};
};

}
