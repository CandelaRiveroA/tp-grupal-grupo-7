#include "farm.h"

#include <algorithm>
#include <utility>

#include "../crops/crop.h"
#include "../crops/green_onion.h"
#include "../crops/potato.h"
#include "../crops/pumpkin.h"
#include "../crops/strawberry.h"
#include "../entities/player.h"
#include "../inventory/item_types.h"

Farm::Farm(uint16_t width, uint16_t height): Scene(width, height) {}

void Farm::on_entity_removed(IEntity* entity) {
    crops.erase(std::remove(crops.begin(), crops.end(), entity), crops.end());
}

void Farm::till_soil(uint16_t x, uint16_t y) {
    if (in_bounds(x, y)) {
        this->grid[y][x].till_soil();
    }
}

void Farm::water_soil(uint16_t x, uint16_t y) {
    if (in_bounds(x, y)) {
        this->grid[y][x].water_soil();
    }
}

bool Farm::try_plant_seed(Player& player, uint16_t tx, uint16_t ty) {
    if (!in_bounds(tx, ty)) {
        return false;
    }
    Cell& target_cell = this->grid[ty][tx];
    if (!target_cell.is_soil_tilled() && !target_cell.is_soil_watered()) {
        return false;
    }

    const Item* active_item = player.peek_equipped_item();
    if (!active_item) {
        return false;
    }

    std::unique_ptr<Crop> new_crop = nullptr;
    switch (active_item->get_id()) {
        case ItemID::STRAWBERRY_SEED: new_crop = std::make_unique<Strawberry>(tx, ty); break;
        case ItemID::ONION_SEED: new_crop = std::make_unique<GreenOnion>(tx, ty); break;
        case ItemID::POTATO_SEED: new_crop = std::make_unique<Potato>(tx, ty); break;
        case ItemID::PUMPKIN_SEED: new_crop = std::make_unique<Pumpkin>(tx, ty); break;
        default: return false;
    }

    crops.push_back(new_crop.get());
    player.consume_equipped_item();
    target_cell.entities.push_back(std::move(new_crop));
    // Si se planta una semilla, el suelo queda arado aunque se haya regado antes.
    target_cell.till_soil();
    return true;
}

void Farm::on_new_day(World& world) {
    const std::vector<Crop*> crops_today = crops;
    for (Crop* crop: crops_today) {
        crop->update(world);
    }

    for (uint16_t y = 0; y < this->height; ++y) {
        for (uint16_t x = 0; x < this->width; ++x) {
            Cell& cell = this->grid[y][x];
            if (cell.is_soil_watered()) {
                cell.dry_soil();
            } else if (cell.is_soil_tilled() && cell.is_empty()) {
                cell.reset_soil();
            }
        }
    }
}

std::size_t Farm::mature_crops() {
    for (Crop* crop: crops) {
        crop->mature();
    }
    return crops.size();
}
