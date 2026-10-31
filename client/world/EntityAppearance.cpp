#include "world/EntityAppearance.h"

#include <cstddef>

namespace world {

AppearanceResolver::AppearanceResolver(const CreatureData& creature)
	: creature_(creature) {}

const ActionData& AppearanceResolver::actionData(Direction facing,
						 ActionKind action) const {
	const DirectionAppearance& appearance =
		creature_.directions[static_cast<int>(facing)];
	const auto it = appearance.actions.find(action);
	if (it != appearance.actions.end()) {
		return it->second;
	}
	return appearance.actions.at(ActionKind::Idle);
}

int AppearanceResolver::frameIndexFor(Direction facing, ActionKind action,
				      int animationTick) const {
	const ActionData& data = actionData(facing, action);
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

EntityAppearance AppearanceResolver::appearanceFor(Direction facing,
						   ActionKind action,
						   int animationTick) const {
	const DirectionAppearance& appearance =
		creature_.directions[static_cast<int>(facing)];
	const ActionData& data = actionData(facing, action);
	const int frame = frameIndexFor(facing, action, animationTick);
	return EntityAppearance{&data.frames[static_cast<std::size_t>(frame)],
				appearance.mirrored};
}

}
