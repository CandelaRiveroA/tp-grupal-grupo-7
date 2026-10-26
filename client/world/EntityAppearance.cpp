#include "world/EntityAppearance.h"

#include <map>
#include <vector>

namespace world {

namespace {

const ActionData& actionData(const CreatureData& creature, Direction facing,
			     ActionKind action) {
	const DirectionAppearance& appearance =
		(*creature.directions)[static_cast<int>(facing)];
	const auto it = appearance.actions->find(action);
	if (it != appearance.actions->end()) {
		return it->second;
	}
	return appearance.actions->at(ActionKind::Idle);
}

}

int frameIndexFor(const CreatureData& creature, Direction facing,
		  ActionKind action, int animationTick) {
	const ActionData& data = actionData(creature, facing, action);
	const int frames = static_cast<int>(data.frames.size());
	if (frames <= 1) {
		return 0;
	}
	const int step = animationTick / data.frameTicks;
	if (data.loops) {
		return step % frames;
	}
	return step < frames ? step : frames - 1;
}

EntityAppearance appearanceFor(const CreatureData& creature,
			       Direction facing, ActionKind action,
			       int animationTick) {
	const DirectionAppearance& appearance =
		(*creature.directions)[static_cast<int>(facing)];
	const ActionData& data = actionData(creature, facing, action);
	const int frame = frameIndexFor(creature, facing, action,
					animationTick);
	return EntityAppearance{&data.frames[static_cast<std::size_t>(frame)],
				appearance.mirrored};
}

}
