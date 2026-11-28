#ifndef SERVER_WORLD_VIEW_H
#define SERVER_WORLD_VIEW_H

#include <cstdint>
#include <string>
#include <vector>

#include "domain/inventory/item_types.h"
#include "domain/map/world.h"

#include "common_server_status.h"

// Vocabulario compartido con el cliente (inventory.toml y resources.toml).
std::string content_id_for(ItemID id);
std::string content_id_for(const Item& item);

// Traduce el estado del mundo a mensajes para un cliente. sync() no guarda
// memoria entre llamadas: cada lote arranca con un ServerReset y describe el
// estado completo, así que lo que no vuelve en el lote ya no existe.
class WorldView {
private:
    ServerInventorySlot inventory_slot(const Inventory& inventory, uint8_t slot) const;

public:
    std::vector<ServerMessage> sync(const World& world, uint8_t player_id) const;
};

#endif  // SERVER_WORLD_VIEW_H
