#include "chest_entity.h"

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
        // Un cofre con ítems adentro absorbe el golpe sin romperse.
        // TODO: Cambiar para que si se rompa y los objetos internos se
        // tiren al piso (como un DroppedItem). Por ahora, solo bloqueamos el golpe.
        if (!this->is_empty()) {
            return; 
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
