#include "world/CreatureCatalog.h"

namespace world {

int CreatureData::toolUseDurationTicks() const {
	for (const DirectionAppearance& direction : directions) {
		const auto it = direction.actions.find(ActionKind::UseTool);
		if (it != direction.actions.end()) {
			const int frames = static_cast<int>(it->second.frames.size());
			const int duration = frames * it->second.frameTicks;
			return duration > 0 ? duration : 1;
		}
	}
	return 1;
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
