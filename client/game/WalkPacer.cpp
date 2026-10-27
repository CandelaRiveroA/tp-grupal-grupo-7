#include "game/WalkPacer.h"

namespace game {

WalkPacer::WalkPacer(const world::MoveCadence& cadence):
        cadence_(cadence),
        lastStepIteration_(0),
        lastDirection_(world::Direction::Front),
        pressedBefore_(false) {}

WalkIntent WalkPacer::intentFor(int iteration, const std::optional<world::Direction>& pressed) {

    if (pressed.has_value()) {
        lastDirection_ = *pressed;

        if (!pressedBefore_) {
            pressedBefore_ = true;
            lastStepIteration_ = iteration;
            return WalkIntent::Step;
        }

        if (iteration - lastStepIteration_ >= cadence_.stepTicks) {
            lastStepIteration_ = iteration;
            return WalkIntent::Step;
        }
        return WalkIntent::None;
    }

    if (pressedBefore_) {
        pressedBefore_ = false;
        return WalkIntent::Stop;
    }

    return WalkIntent::None;
}

world::Direction WalkPacer::lastDirection() const { return lastDirection_; }

}  // namespace game
