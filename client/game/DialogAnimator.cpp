#include "game/DialogAnimator.h"

#include <utility>

namespace game {

void DialogAnimator::observe(const LocalWorldView& view, const int iteration) {
	std::map<int, int> updated;
	for (const ViewEntity& entity : view.entities) {
		if (!entity.indication.has_value()) {
			continue;
		}
		const auto it = startByEntityId_.find(entity.id);
		// Ya estaba mostrando: conserva su tick de aparición. Si es
		// nueva (o reaparece), arranca la animación de cero.
		updated[entity.id] =
			it != startByEntityId_.end() ? it->second : iteration;
	}
	startByEntityId_ = std::move(updated);
}

int DialogAnimator::animationTickFor(const ViewEntity& entity,
				     const int iteration) const {
	if (!entity.indication.has_value()) {
		return 0;
	}
	const auto it = startByEntityId_.find(entity.id);
	if (it == startByEntityId_.end()) {
		return 0;
	}
	return iteration - it->second;
}

}  // namespace game
