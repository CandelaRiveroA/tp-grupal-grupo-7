#include "world/CreatureCatalog.h"

namespace world {

int CreatureData::actionWindowTicks(const ActionKind action) const {
	const int duration = actionCycleTicks(action);
	return duration > 0 ? duration : 1;
}

int CreatureData::toolUseDurationTicks() const {
	return actionWindowTicks(ActionKind::UseTool);
}

int CreatureData::actionCycleTicks(ActionKind action) const {
	for (const DirectionAppearance& direction : directions) {
		const auto it = direction.actions.find(action);
		if (it != direction.actions.end()) {
			const int frames = static_cast<int>(it->second.frames.size());
			const int duration = frames * it->second.frameTicks;
			return duration > 0 ? duration : 1;
		}
	}
	return 0;
}

}  // namespace world
