#include "game/ActionStartCue.h"

namespace game {

std::vector<ActionStart> ActionStartCue::update(const LocalWorldView& view) {
	std::vector<ActionStart> starts;
	for (const ViewEntity& entity : view.entities) {
		const auto previous = lastActions_.find(entity.id);
		const bool isStart = previous == lastActions_.end() ||
				     previous->second != entity.action;
		lastActions_[entity.id] = entity.action;
		if (isStart) {
			starts.push_back(ActionStart{entity.id, entity.kind,
						     entity.action,
						     entity.facing});
		}
	}
	return starts;
}

}  // namespace game
