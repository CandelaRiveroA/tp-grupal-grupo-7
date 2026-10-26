#pragma once

#include <optional>

#include "common/common_game_direction.h"
#include "world/CreatureCatalog.h"

namespace game {

enum class WalkIntent {
	None,
	Step,
	Stop,
};

class WalkPacer {
public:
	explicit WalkPacer(const world::MoveCadence& cadence);

	WalkIntent intentFor(int iteration,
			     const std::optional<protocol::Direction>& pressed);

	protocol::Direction lastDirection() const;

private:
	world::MoveCadence cadence_;

	int lastStepIteration_;

	protocol::Direction lastDirection_;

	bool pressedBefore_;
};

}
