#include "game/ObjectPicker.h"

#include "world/DroppedItemLayout.h"
#include "world/WorldConstants.h"

namespace game {

ObjectPicker::ObjectPicker(const world::ContentCatalog& content,
			   const world::FurnitureGeometry& furniture)
	: content_(content), furniture_(furniture) {}

PickTarget ObjectPicker::pick(const world::PixelPoint& worldPoint,
			      const LocalWorldView& view,
			      const ViewMotion& motion, const int iteration,
			      const int cellPixelSize) const {
	// Entidades arriba de los ítems del piso (orden de dibujo); dentro
	// de cada lista, la última dibujada es la de más arriba.
	for (std::size_t i = view.entities.size(); i > 0; --i) {
		const std::size_t index = i - 1;
		const world::PixelRect box = motion.drawBoxFor(
			view.entities[index], index, iteration, cellPixelSize);
		if (contains(box, worldPoint)) {
			return PickTarget{PickTarget::Kind::Entity, index};
		}
	}

	const world::ScenePixelMapper mapper(cellPixelSize);
	const world::DroppedItemLayout items;
	for (std::size_t i = view.droppedItems.size(); i > 0; --i) {
		const std::size_t index = i - 1;
		const ViewDroppedItem& item = view.droppedItems[index];
		const world::SpriteVisual* visual = content_.find(item.contentId);
		if (visual == nullptr) {
			continue;
		}
		const world::PixelRect box = items.rectFor(
			mapper.toPixels(world::GridCell{item.cellX, item.cellY},
					world::kUnitCellSpan),
			*visual);
		if (contains(box, worldPoint)) {
			return PickTarget{PickTarget::Kind::DroppedItem, index};
		}
	}

	// Cultivos plantados al fondo (orden de dibujo): su caja es la
	// celda de tierra donde se dibuja la planta.
	for (std::size_t i = view.plantedCrops.size(); i > 0; --i) {
		const std::size_t index = i - 1;
		const ViewCrop& crop = view.plantedCrops[index];
		const world::PixelRect box = mapper.toPixels(
			world::GridCell{crop.cellX, crop.cellY},
			world::kUnitCellSpan);
		if (contains(box, worldPoint)) {
			return PickTarget{PickTarget::Kind::Crop, index};
		}
	}

	// Muebles colocados al fondo: la caja es la de su sprite entero
	// (todas sus partes: tapa y base cuentan como UNA unidad).
	for (std::size_t i = view.placedFurniture.size(); i > 0; --i) {
		const std::size_t index = i - 1;
		const ViewFurniture& piece = view.placedFurniture[index];
		const std::optional<world::FurnitureShape> shape = furniture_.shapeOf(
			piece.contentId, world::GridCell{piece.cellX, piece.cellY}, mapper,
			piece.facing);
		if (shape.has_value() && contains(shape->box, worldPoint)) {
			return PickTarget{PickTarget::Kind::Furniture, index};
		}
	}

	return PickTarget{};
}

bool ObjectPicker::contains(const world::PixelRect& box,
			    const world::PixelPoint& point) const {
	return point.x >= box.x && point.x < box.x + box.width &&
	       point.y >= box.y && point.y < box.y + box.height;
}

}  // namespace game
