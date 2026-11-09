#include "world_view.h"

#include <map>
#include <optional>

#include "domain/inventory/inventory.h"
#include "domain/inventory/item.h"

// Vocabulario compartido con el cliente (inventory.toml y resources.toml).
std::string content_id_for(ItemID id) {
    static const std::map<ItemID, std::string> content_ids = {
            {ItemID::PICKAXE, "pickaxe"},
            {ItemID::SHOVEL, "shovel"},
            {ItemID::WATERING_CAN, "watering_can"},
            {ItemID::SWORD, "sword"},
            {ItemID::SILVER_ORB, "silver"},
            {ItemID::COAL_ORB, "coal"},
            {ItemID::LAPIS_ORB, "lapis"},
            {ItemID::GOLD_ORB, "gold"},
            {ItemID::ROCK, "rock"},
            {ItemID::EGG, "egg"},
            {ItemID::MILK, "milk"},
            {ItemID::BED, "bed"}};

    const auto it = content_ids.find(id);
    return it != content_ids.end() ? it->second : "";
}

static ResourceDamage damage_for(uint8_t hits_taken, uint8_t hits_to_destroy) {
    if (hits_taken == 0) {
        return ResourceDamage::Intact;
    }

    if (static_cast<unsigned>(hits_taken) + 1 >= hits_to_destroy) {
        return ResourceDamage::Broken;
    }

    return ResourceDamage::Cracked;
}

std::vector<ServerMessage> WorldView::sync(const World& world, uint8_t player_id) const {
    std::vector<ServerMessage> out;
    // El reset va primero: sin él el cliente arrastraría celdas, slots y
    // posiciones que el lote ya no vuelve a describir.
    out.push_back(ServerReset{});

    for (const uint8_t id: world.get_player_ids()) {
        const Player& player = world.get_player(id);
        out.push_back(ServerStatus(id, player.get_x(), player.get_y(), player.get_facing()));
    }

    for (uint16_t y = 0; y < world.get_height(); ++y) {
        for (uint16_t x = 0; x < world.get_width(); ++x) {
            for (const IEntity* entity: world.get_entity_at(x, y)) {
                if (const auto animal = entity->animal_snapshot()) {
                    out.push_back(ServerAnimal{animal->id, animal->kind, x, y, animal->facing,
                                               animal->ready_to_milk});
                    continue;
                }
                const auto snap = entity->snapshot();
                if (!snap) {
                    continue;
                }
                if (snap->kind == CellSnapshot::Kind::Resource) {
                    out.push_back(
                            ServerResource{x, y, content_id_for(snap->item),
                                           damage_for(snap->hits_taken, snap->hits_to_destroy)});
                } else if (snap->kind == CellSnapshot::Kind::Furniture) {
                    // El mueble va como ServerFurniture: el cliente no debe
                    // tratarlo como un ítem del piso recogible.
                    out.push_back(ServerFurniture{x, y, content_id_for(snap->item)});
                } else {
                    const bool interactive = entity->pickup_on_take();
                    out.push_back(ServerDropped{x, y, content_id_for(snap->item), interactive});
                }
            }
        }
    }

    // Los slots vacíos no se mandan: el reset ya los borró del cliente.
    const Inventory& inventory = world.get_player(player_id).get_inventory();
    for (uint8_t slot = 0; slot < inventory.get_max_slots(); ++slot) {
        const Item* item = inventory.peek_item(slot);
        if (item != nullptr) {
            out.push_back(ServerInventorySlot{slot, inventory.get_amount(slot),
                                              content_id_for(item->get_id())});
        }
    }
    // Cierra el lote: el cliente aplica todo recién al ver este mensaje.
    out.push_back(ServerSyncEnd{});
    return out;
}
