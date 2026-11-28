#include "chest_entity.h"
#include "../entities/dropped_item.h"
#include "../map/world.h"

ChestEntity::ChestEntity(uint16_t x, uint16_t y, ItemID drop_id, uint16_t model_id, uint8_t facing)
    : Furniture(x, y, drop_id, model_id), storage(CHEST_INVENTORY_SIZE), facing(facing) {}

Inventory& ChestEntity::get_inventory() { 
    return this->storage; 
}

const Inventory& ChestEntity::get_inventory() const { 
    return this->storage; 
}

bool ChestEntity::is_empty() const {
    for (uint8_t i = 0; i < CHEST_INVENTORY_SIZE; ++i) {
        if (!this->storage.is_slot_empty(i)) {
            return false;
        }
    }
    return true;
}

void ChestEntity::interact(Player& p, ToolType tool, World& w) {
    if (tool == ToolType::HAND) {
        on_use(p, w);
        return;
    }
    if (tool == ToolType::PICKAXE && w.container_in_use(w.get_active_scene(), get_x(), get_y())) {
        return;
    }
    if (tool == ToolType::PICKAXE) {
        // Recorremos los 30 espacios posibles del cofre
        for (uint8_t i = 0; i < CHEST_INVENTORY_SIZE; ++i) {
            
            while (!this->storage.is_slot_empty(i)) {
                
                // 1. Extraemos 1 sola unidad
                std::unique_ptr<Item> unidad_extraida = this->storage.remove_item(i);
                
                // 2. La envolvemos en su DroppedItem físico en las mismas coordenadas del cofre
                auto drop = std::make_unique<DroppedItem>(
                    std::move(unidad_extraida), 
                    this->pos_x, 
                    this->pos_y
                );
                
                // 3. El mundo la inserta en el std::vector de la celda,
                // conviviendo pacíficamente en la misma coordenada O(1)
                w.set_entity_at(this->pos_x, this->pos_y, std::move(drop));
            }
        }
    }
    
    Furniture::interact(p, tool, w);
}

void ChestEntity::on_use(Player& p, World& w) {
    w.open_container(p.get_id(), get_x(), get_y());
}

Inventory* ChestEntity::container_inventory() { return &storage; }
const Inventory* ChestEntity::container_inventory() const { return &storage; }
uint8_t ChestEntity::furniture_facing() const { return facing; }
