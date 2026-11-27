#pragma once

#include <cstddef>

#include "game/LocalWorldView.h"
#include "game/ViewMotion.h"
#include "world/ContentCatalog.h"
#include "world/FurnitureGeometry.h"
#include "world/ScenePixelMapper.h"

namespace game {

// Qué hay bajo un punto del mundo: identificador NEUTRO de lo picked
// (solo datos). Qué hacer con él lo decide quien llama.
struct PickTarget {
	enum class Kind {
		None,
		Entity,
		DroppedItem,
		Crop,
		Furniture,
	};

	Kind kind = Kind::None;

	// Índice en la lista de la vista (entities / droppedItems /
	// plantedCrops /
	// placedFurniture).
	std::size_t index = 0;
};

// Geometría pura del pick: qué hay bajo un punto del mundo, sean
// entidades, ítems del piso o cultivos plantados. Usa las MISMAS cajas
// dibujadas que el renderer (ViewMotion::drawBoxFor, DroppedItemLayout
// y la celda plantada), así lo que se puede tocar coincide siempre con
// lo que se ve. No conoce SDL ni dibuja.
class ObjectPicker {
public:
	// El catálogo define cómo se dibuja cada ítem del piso (por
	// contentId, con su tamaño y ajuste al suelo); el pick lo necesita
	// para medir su caja igual que el pintor.
	// `furniture`: la geometría de los muebles colocados (la misma del
	// dibujo).
	ObjectPicker(const world::ContentCatalog& content,
		     const world::FurnitureGeometry& furniture);

	// Objetivo bajo el punto de mundo (Kind::None si no hay ninguno).
	// Si varios se superponen, gana el de más arriba en el orden de
	// dibujo (entidades sobre ítems y cultivos; ítems sobre cultivos;
	// dentro de cada lista, la última dibujada primero).
	PickTarget pick(const world::PixelPoint& worldPoint,
			const LocalWorldView& view, const ViewMotion& motion,
			int iteration, int cellPixelSize) const;

private:
	bool contains(const world::PixelRect& box,
		      const world::PixelPoint& point) const;

	const world::ContentCatalog& content_;
	const world::FurnitureGeometry& furniture_;
};

}  // namespace game
