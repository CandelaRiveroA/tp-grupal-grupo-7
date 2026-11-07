#pragma once

#include <functional>

#include "core/Constants.h"
#include "../common/common_frame_clock.h"

namespace sdl {
class InputManager;
}

namespace sdl {

class GameLoop {
public:
    GameLoop(sdl::InputManager& input, std::function<void(int)> tick,
             int targetFps = core::kTargetFps);

    void run();
    void stop();

private:
    sdl::InputManager& input_;
    std::function<void(int)> tick_;
    FrameClock clock_;
    bool running_;
};

}  // namespace sdl
