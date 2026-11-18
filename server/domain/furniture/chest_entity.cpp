#include "chest_entity.h"
#include "../entities/dropped_item.h"
#include "../map/world.h"

ChestEntity::ChestEntity(uint16_t x, uint16_t y, ItemID drop_id, uint16_t model_id)
    : Furniture(x, y, drop_id, model_id), storage(CHEST_INVENTORY_SIZE) {}

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
    // Al usarse con las manos, el cofre no hace modificaciones locales.
    // Solo le avisa al World que este Player quiere abrir este cofre.
    // El World debería encolar un evento para que el servidor le mande al cliente
    // el DTO del inventario del cofre y abra la interfaz gráfica.
}
