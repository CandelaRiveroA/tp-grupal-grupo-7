#pragma once

#include "common/common_constants.h"

namespace world {

enum class Direction {
    Front,
    Back,
    Left,
    Right,
};

inline Direction viewDirectionFrom(::Direction direction) {
    switch (direction) {
        case ::Direction::UP:
            return Direction::Back;
        case ::Direction::DOWN:
            return Direction::Front;
        case ::Direction::LEFT:
            return Direction::Left;
        case ::Direction::RIGHT:
            return Direction::Right;
    }
    return Direction::Front;
}

inline ::Direction wireDirectionFrom(Direction direction) {
    switch (direction) {
        case Direction::Back:
            return ::Direction::UP;
        case Direction::Front:
            return ::Direction::DOWN;
        case Direction::Left:
            return ::Direction::LEFT;
        case Direction::Right:
            return ::Direction::RIGHT;
    }
    return ::Direction::DOWN;
}

}  // namespace world
