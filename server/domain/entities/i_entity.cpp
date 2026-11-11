#include "i_entity.h"

IEntity::IEntity(uint16_t x, uint16_t y): pos_x(x), pos_y(y) {}

uint16_t IEntity::get_x() const { return this->pos_x; }

uint16_t IEntity::get_y() const { return this->pos_y; }

void IEntity::set_position(uint16_t x, uint16_t y) {
    this->pos_x = x;
    this->pos_y = y;
}

bool IEntity::is_walkable() const {
    return false;  
}

bool IEntity::blocks_placement() const {
    return false;  
}

bool IEntity::pickup_on_take() const { return false; }

std::optional<CellSnapshot> IEntity::snapshot() const { return std::nullopt; }

std::optional<AnimalSnapshot> IEntity::animal_snapshot() const { return std::nullopt; }

std::optional<CropSnapshot> IEntity::crop_snapshot() const { return std::nullopt; }

void IEntity::update(World& /*w*/) {}
