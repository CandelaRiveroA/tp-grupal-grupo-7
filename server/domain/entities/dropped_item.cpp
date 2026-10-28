#include "dropped_item.h"
#include "player.h"
#include "../map/world.h"

DroppedItem::DroppedItem(std::unique_ptr<Item> item, uint16_t x, uint16_t y)
    : IEntity(x, y), payload(std::move(item)) {}

bool DroppedItem::is_walkable() const {
    return true;
}

void DroppedItem::interact(Player& p, ToolType tool, World& w) {
    // Dejamos la mano para los huevos
    // Pero tal bien se pueden levantar solo caminando.
    if (tool == ToolType::HAND && this->payload) {
        uint16_t cx = this->pos_x;
        uint16_t cy = this->pos_y;

        if (p.pick_up_item(std::move(this->payload))) {
            w.remove_entity_at(cx, cy);
        }
    }
}