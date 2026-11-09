#include "game/DialogAnimator.h"

#include <utility>

namespace game {

void DialogAnimator::observe(const LocalWorldView& view, const int iteration) {
	// Entidades y cultivos comparten el mismo mecanismo; cada tick se
	// reconstruye la lista de dueños que muestran diálogo (lo que no
	// muestra se olvida).
	std::map<int, int> entities;
	for (const ViewEntity& entity : view.entities) {
		if (!entity.indication.has_value()) {
			continue;
		}
		carryStart(entities, startByEntityId_, entity.id, iteration);
	}
	startByEntityId_ = std::move(entities);

	std::map<world::GridCell, int> crops;
	for (const ViewCrop& crop : view.plantedCrops) {
		if (!crop.indication.has_value()) {
			continue;
		}
		carryStart(crops, startByCropCell_,
			   world::GridCell{crop.cellX, crop.cellY}, iteration);
	}
	startByCropCell_ = std::move(crops);
}

int DialogAnimator::animationTickFor(const ViewEntity& entity,
				     const int iteration) const {
	return tickFor(startByEntityId_, entity.id,
		       entity.indication.has_value(), iteration);
}

int DialogAnimator::animationTickFor(const ViewCrop& crop,
				     const int iteration) const {
	return tickFor(startByCropCell_,
		       world::GridCell{crop.cellX, crop.cellY},
		       crop.indication.has_value(), iteration);
}

}
