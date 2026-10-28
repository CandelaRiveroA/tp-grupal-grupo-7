#pragma once

#include <optional>
#include <string>
#include <vector>

#include "world/ViewDirection.h"
#include "world/CreatureCatalog.h"
#include "world/Grid.h"
#include "world/ItemCatalog.h"
#include "world/ResourceCatalog.h"

namespace game {


struct ViewEntity {
	world::CreatureKind kind;
	int cellX;
	int cellY;
	world::Direction facing;
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

// Un slot del inventario. El inventario es GENÉRICO: guarda cualquier
// contenido (hoy herramientas y minerales, mañana huevos u otros) mediante
// un identificador abstracto. La fuente (servidor) decide qué contentId va
// en cada slot y en qué cantidad; el cliente resuelve contentId -> asset
// (world::ContentCatalog) y dibuja. No depende de ItemKind.
//
// contentId: clave estable del contenido, vocabulario compartido con el
//            servidor (ej. "pickaxe", "silver", futuro "egg").
// count:     cantidad acumulada en el slot. Convención de dibujo: 1 no
//            muestra número; >1 muestra el número.
struct ViewInventorySlot {
	int slotIndex;
	std::string contentId;
	int count;
};

// Evento puntual del tick: la fuente informa que el jugador OBTUVO un
// recurso (no lo deduce el cliente). Se limpia cada tick.
struct ViewResourceObtained {
	world::ResourceKind kind;
};

// Item caído al piso, recogible por el jugador. GENÉRICO: lo identifica un
// contentId (string, mismo vocabulario que el inventario y el servidor), no
// un tipo concreto. Hoy caen minerales; mañana huevos u otros, sin cambiar
// esta estructura. La fuente decide contentId y celda (no se deduce); el
// cliente lo dibuja en el piso resolviendo contentId -> asset
// (world::ContentCatalog). Al recogerse (comando Take sobre su celda) sale
// del vector y se emite obtained.
struct ViewDroppedItem {
	std::string contentId;
	int cellX;
	int cellY;
};

struct LocalWorldView {
	std::vector<ViewEntity> entities;
	std::vector<ViewResource> resources;
	std::vector<ViewResourceObtained> obtained;
	std::vector<ViewDroppedItem> droppedItems;
	std::vector<ViewInventorySlot> inventory;
	world::GridSpan grid{0, 0};
};

}
