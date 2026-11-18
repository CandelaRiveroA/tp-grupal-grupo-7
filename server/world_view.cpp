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
            {ItemID::ROCK_2, "rock_2"},
            {ItemID::ROCK_3, "rock_3"},
            {ItemID::ROCK_4, "rock_4"},
            {ItemID::ROCK_5, "rock_5"},
            {ItemID::ROCK_6, "rock_6"},
            {ItemID::EGG, "egg"},
            {ItemID::MILK, "milk"},
            {ItemID::BED, "bed"},
            {ItemID::STRAWBERRY, "strawberry"},
            {ItemID::GREEN_ONION, "onion"},
            {ItemID::POTATO, "potato"},
            {ItemID::PUMPKIN, "pumpkin"},
            {ItemID::STRAWBERRY_SEED, "strawberry_seeds"},
            {ItemID::ONION_SEED, "onion_seeds"},
            {ItemID::POTATO_SEED, "potato_seeds"},
            {ItemID::PUMPKIN_SEED, "pumpkin_seeds"}};

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
        out.push_back(ServerStatus(id, player.get_x(), player.get_y(), player.get_facing(),
                                   player.get_appearance(),
                                   player.get_health(), player.get_max_health(),
                                   static_cast<uint16_t>(player.get_energy() * 100.0f),
                                   static_cast<uint16_t>(player.get_max_energy() * 100.0f),
                                    player.faintedByEnergy() ? 1u : 0u));
    }

    for (uint16_t y = 0; y < world.get_height(); ++y) {
        for (uint16_t x = 0; x < world.get_width(); ++x) {
            // El suelo no es entidad: su estado es la celda misma (Cell::soil_state).
            const SoilState soil = world.get_cell(x, y).soil_state;
            if (soil != SoilState::NORMAL) {
                out.push_back(ServerGroundEffect{x, y, GroundEffect::Dug});
            }
            if (soil == SoilState::WATERED) {
                out.push_back(ServerGroundEffect{x, y, GroundEffect::Watered});
            }
            for (const IEntity* entity: world.get_entity_at(x, y)) {
                if (const auto animal = entity->animal_snapshot()) {
                    out.push_back(ServerAnimal{animal->id, animal->kind, x, y, animal->facing,
                                               animal->ready_to_milk});
                    continue;
                }
                if (const auto enemy = entity->enemy_snapshot()) {
                    out.push_back(ServerEnemy{enemy->id, enemy->kind, x, y, enemy->facing,
                                              enemy->health, enemy->action});
                    continue;
                }
                if (const auto crop = entity->crop_snapshot()) {
                    out.push_back(ServerCrop{x, y, content_id_for(crop->product), crop->stage,
                                             crop->needs_water, crop->ready,
                                             static_cast<uint8_t>(crop->quality)});
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

    // Charcos de sangre: estado persistente del dia (se secan al terminar
    // el dia o los consume una vampira). Van en el MISMO transporte que la
    // tierra tratada, asi el cliente los pinta bajo las entidades.
    for (const auto& [blood_x, blood_y]: world.blood_puddle_list()) {
        out.push_back(ServerGroundEffect{blood_x, blood_y, GroundEffect::Blood});
    }

    // Las llamas van después de las celdas (el fuego se dibuja encima), en
    // el mismo orden que EnemyBot::appendEntities.
    for (const ProjectileSnapshot& projectile: world.projectile_snapshots()) {
        out.push_back(ServerProjectile{projectile.id, projectile.kind, projectile.x, projectile.y,
                                       projectile.direction});
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
