#include "test_helpers.h"

#include <vector>

bool has_item(const Player& player, ItemID id) {
    const Inventory& inventory = player.get_inventory();
    for (uint8_t slot = 0; slot < inventory.get_max_slots(); ++slot) {
        const Item* item = inventory.peek_item(slot);
        if (item != nullptr && item->get_id() == id) {
            return true;
        }
    }
    return false;
}

bool cell_has_item(World& world, uint16_t x, uint16_t y, ItemID id) {
    std::vector<const IEntity*> entities;
    for (const IEntity* e : world.get_scene(SceneId::FARMHOUSE).get_entity_at(x, y)) {
        entities.push_back(e);
    }
    for (const IEntity* e : world.get_farm().get_entity_at(x, y)) {
        entities.push_back(e);
    }
    for (const IEntity* entity : entities) {
        const auto snap = entity->snapshot();
        if (snap && snap->item == id) {
            return true;
        }
    }
    return false;
}

bool cell_has_furniture(const World& world, uint16_t x, uint16_t y) {
    for (const IEntity* entity : world.get_entity_at(x, y)) {
        const auto snap = entity->snapshot();
        if (snap && snap->kind == CellSnapshot::Kind::Furniture) {
            return true;
        }
    }
    return false;
}