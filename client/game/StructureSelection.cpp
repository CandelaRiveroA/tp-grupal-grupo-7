#include "game/StructureSelection.h"

#include "game/AreaProximity.h"
#include "game/GameConstants.h"
#include "world/ElementLayout.h"

namespace game {

StructureSelection::StructureSelection(const world::SceneDescription& scene,
				       int cellPixelSize) {
	reindex(scene, cellPixelSize);
}

void StructureSelection::reindex(const world::SceneDescription& scene,
				 int cellPixelSize) {
	entries_.clear();
	const world::ScenePixelMapper mapper(cellPixelSize);
	for (const world::SceneStructure& structure : scene.structures) {
		if (structure.action == world::StructureAction::None ||
		    structure.elementIndex >= scene.elements.size()) {
			continue;
		}
		// La misma geometría que usa el renderer para el elemento:
		// lo que se toca es lo que se dibuja.
		const world::ElementPlacement& element =
			scene.elements[structure.elementIndex];
		const world::ElementLayout layout(
			scene.tilesets.at(element.tileset), element,
			mapper.toPixels(element.at, element.span));
		entries_.push_back(Entry{layout.bounds(), structure});
	}
}

StructureClick StructureSelection::clickAt(
	const world::PixelPoint& worldPoint, const LocalWorldView& view) const {
	StructureClick click;
	const Entry* entry = entryAt(worldPoint);
	if (entry == nullptr) {
		return click;
	}
	click.hit = true;
	click.structure = &entry->structure;
	click.inRange = near(*entry, view);
	return click;
}

bool StructureSelection::hits(const world::PixelPoint& worldPoint,
			      const LocalWorldView& view) const {
	const Entry* entry = entryAt(worldPoint);
	return entry != nullptr && near(*entry, view);
}

const StructureSelection::Entry* StructureSelection::entryAt(
	const world::PixelPoint& worldPoint) const {
	for (std::size_t i = entries_.size(); i > 0; --i) {
		const world::PixelRect& box = entries_[i - 1].bounds;
		if (worldPoint.x >= box.x && worldPoint.x < box.x + box.width &&
		    worldPoint.y >= box.y && worldPoint.y < box.y + box.height) {
			return &entries_[i - 1];
		}
	}
	return nullptr;
}

bool StructureSelection::near(const Entry& entry,
			      const LocalWorldView& view) const {
	if (view.entities.empty()) {
		return false;
	}
	const ViewEntity& local = view.entities[kLocalPlayerIndex];
	return AreaProximity(kInteractRing)
		.around(entry.structure.at, entry.structure.span, local.cellX,
			local.cellY);
}

}
