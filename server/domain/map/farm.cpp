#include "farm.h"

#include <algorithm>
#include <cstdlib>
#include <utility>

#include "../animals/animal.h"
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
    animals.erase(std::remove(animals.begin(), animals.end(), entity), animals.end());
}

void Farm::add_animal(std::unique_ptr<Animal> animal) {
    if (!animal) {
        return;
    }
    const uint16_t x = animal->get_x();
    const uint16_t y = animal->get_y();
    if (!in_bounds(x, y)) {
        return;
    }
    set_entity_at(x, y, std::move(animal));
    animals.push_back(static_cast<Animal*>(this->grid[y][x].entities.back().get()));
}

void Farm::tick_animals(World& world) {
    for (Animal* animal: animals) {
        animal->wander(world);
    }
}

bool Farm::try_hand_action_animals(Player& player, World& world) {
    for (Animal* animal: animals) {
        const int dx = static_cast<int>(animal->get_x()) - static_cast<int>(player.get_x());
        const int dy = static_cast<int>(animal->get_y()) - static_cast<int>(player.get_y());
        if (std::abs(dx) <= 1 && std::abs(dy) <= 1 && animal->try_hand_action(player, world)) {
            return true;
        }
    }
    return false;
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
    const std::vector<Animal*> animals_today = animals;
    for (Animal* animal: animals_today) {
        animal->update(world);
    }

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
