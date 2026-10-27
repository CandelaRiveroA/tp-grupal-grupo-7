#pragma once

#include <optional>

#include "world/CreatureCatalog.h"
#include "world/ViewDirection.h"

namespace game {

enum class WalkIntent {
    None,
    Step,
    Stop,
};

class WalkPacer {
public:
    explicit WalkPacer(const world::MoveCadence& cadence);

    WalkIntent intentFor(int iteration, const std::optional<world::Direction>& pressed);

    world::Direction lastDirection() const;

private:
    world::MoveCadence cadence_;

    int lastStepIteration_;

    world::Direction lastDirection_;

    bool pressedBefore_;
};

}  // namespace game
