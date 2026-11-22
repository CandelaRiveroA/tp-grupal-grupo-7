#include "i_movable.h"

#include <unordered_map>

// Desplazamiento (dx, dy) de cada dirección. Arriba e izquierda restan, y el
// borde inferior o derecho lo frena can_walk_to del mundo.
static const std::unordered_map<Direction, std::pair<int, int>> kSteps = {
        {Direction::UP, {0, -1}},
        {Direction::DOWN, {0, 1}},
        {Direction::LEFT, {-1, 0}},
        {Direction::RIGHT, {1, 0}},
};

std::pair<uint16_t, uint16_t> IMovable::next_tile(uint16_t x, uint16_t y, Direction dir) {
    const auto [dx, dy] = kSteps.at(dir);
    const int next_x = x + dx;
    const int next_y = y + dy;
    if (next_x < 0 || next_y < 0) {
        return {x, y};
    }
    return {static_cast<uint16_t>(next_x), static_cast<uint16_t>(next_y)};
}
