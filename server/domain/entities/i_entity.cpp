#include "i_entity.h"

IEntity::IEntity(uint16_t x, uint16_t y) : pos_x(x), pos_y(y) {}

uint16_t IEntity::get_x() const { return this->pos_x; }

uint16_t IEntity::get_y() const { return this->pos_y; }

void IEntity::set_position(uint16_t x, uint16_t y) {
    this->pos_x = x;
    this->pos_y = y;
}

bool IEntity::is_walkable() const {
    return false; // Por defecto bloquean el paso
}

void IEntity::update() {
    // Por defecto no hace nada (útil para minerales).
    // Los NPC o cultivos lo sobreescribirán.
}
