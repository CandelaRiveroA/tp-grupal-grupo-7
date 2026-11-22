#include "entity_controller.h"

#include <algorithm>
#include <utility>
#include <vector>

#include "../map/world.h"

EntityController::EntityController(uint8_t density_percent, uint32_t seed):
        rng(seed), density_percent(density_percent) {}

void EntityController::populate(World& world, uint16_t reserved_x, uint16_t reserved_y) {
    clear_generation(world);

    std::vector<std::pair<uint16_t, uint16_t>> free_cells;
    for (uint16_t y = 0; y < world.get_height(); ++y) {
        for (uint16_t x = 0; x < world.get_width(); ++x) {
            if (x == reserved_x && y == reserved_y) {
                continue;
            }
            if (world.get_entity_at(x, y).empty()) {
                free_cells.emplace_back(x, y);
            }
        }
    }

    std::shuffle(free_cells.begin(), free_cells.end(), rng);
    const size_t count = free_cells.size() * this->density_percent / 100;
    for (size_t i = 0; i < count; ++i) {
        const auto [x, y] = free_cells[i];
        // Una manada o pareja puede haber ocupado la celda mientras se generaba.
        if (!world.get_entity_at(x, y).empty()) {
            continue;
        }
        spawn_at(world, x, y);
    }
}
