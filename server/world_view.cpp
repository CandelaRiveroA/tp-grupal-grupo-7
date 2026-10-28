#include "world_view.h"

#include <cstddef>
#include <utility>

#include "domain/inventory/inventory.h"
#include "domain/inventory/item.h"

namespace {

// Vocabulario compartido con el cliente (inventory.toml y resources.toml).
std::string content_id_for(ItemID id) {
    static const std::map<ItemID, std::string> content_ids = {
            {ItemID::PICKAXE, "pickaxe"},           {ItemID::SHOVEL, "shovel"},
            {ItemID::WATERING_CAN, "watering_can"}, {ItemID::SWORD, "sword"},
            {ItemID::SILVER_ORB, "silver"},         {ItemID::COAL_ORB, "coal"},
            {ItemID::LAPIS_ORB, "lapis"},           {ItemID::GOLD_ORB, "gold"}};

    const auto it = content_ids.find(id);
    return it != content_ids.end() ? it->second : "";
}

ResourceDamage damage_for(uint8_t hits_taken, uint8_t hits_to_destroy) {
    if (hits_taken == 0) {
        return ResourceDamage::Intact;
    }

    if (static_cast<unsigned>(hits_taken) + 1 >= hits_to_destroy) {
        return ResourceDamage::Broken;
    }

    return ResourceDamage::Cracked;
}

}  // namespace

std::vector<ServerMessage> WorldView::sync(const World& world, uint8_t player_id) {
    std::vector<ServerMessage> out;
    PlayerView& view = views[player_id];
    sync_poses(world, view, out);
    sync_cells(world, view, out);
    sync_inventory(world, player_id, view, out);
    view.synced = true;
    return out;
}

void WorldView::forget(uint8_t player_id) { views.erase(player_id); }

void WorldView::sync_poses(const World& world, PlayerView& view, std::vector<ServerMessage>& out) {
    std::map<uint8_t, PlayerPose> current;
    for (const uint8_t id: world.get_player_ids()) {
        const Player& player = world.get_player(id);
        current[id] = PlayerPose{player.get_x(), player.get_y(), player.get_facing()};
    }

    for (const auto& [id, pose]: current) {
        const auto it = view.poses.find(id);
        if (it == view.poses.end() || !(it->second == pose)) {
            out.push_back(ServerStatus(id, pose.x, pose.y, pose.direction));
        }
    }
    view.poses = std::move(current);
}

void WorldView::sync_cells(const World& world, PlayerView& view, std::vector<ServerMessage>& out) {
    std::map<Position, CellContent> current;
    for (uint16_t y = 0; y < world.get_height(); ++y) {
        for (uint16_t x = 0; x < world.get_width(); ++x) {
            const IEntity* entity = world.get_entity_at(x, y);
            const auto snap = entity ? entity->snapshot() : std::nullopt;
            if (!snap) {
                continue;
            }
            const bool is_resource = snap->kind == CellSnapshot::Kind::Resource;
            current[Position{x, y}] =
                    CellContent{is_resource, content_id_for(snap->item),
                                is_resource ? damage_for(snap->hits_taken, snap->hits_to_destroy) :
                                              ResourceDamage::Intact};
        }
    }

    for (const auto& [pos, old]: view.cells) {
        const auto it = current.find(pos);
        // Si la celda sigue siendo del mismo tipo, el mensaje de alta la reemplaza.
        if (it != current.end() && it->second.is_resource == old.is_resource) {
            continue;
        }
        if (old.is_resource) {
            out.push_back(ServerResourceRemoved{pos.x, pos.y});
        } else {
            out.push_back(ServerDroppedRemoved{pos.x, pos.y});
        }
    }

    for (const auto& [pos, now]: current) {
        const auto it = view.cells.find(pos);
        if (it != view.cells.end() && it->second == now) {
            continue;
        }
        if (now.is_resource) {
            out.push_back(ServerResource{pos.x, pos.y, now.content_id, now.damage});
        } else {
            out.push_back(ServerDropped{pos.x, pos.y, now.content_id});
        }
    }
    view.cells = std::move(current);
}

void WorldView::sync_inventory(const World& world, uint8_t player_id, PlayerView& view,
                               std::vector<ServerMessage>& out) {
    const Inventory& inventory = world.get_player(player_id).get_inventory();
    std::vector<SlotContent> current;
    for (uint8_t slot = 0; slot < inventory.get_max_slots(); ++slot) {
        const Item* item = inventory.peek_item(slot);
        if (item == nullptr) {
            current.push_back(SlotContent{"", 0});
        } else {
            current.push_back(
                    SlotContent{content_id_for(item->get_id()), inventory.get_amount(slot)});
        }
    }
    if (view.slots.size() != current.size()) {
        view.slots.assign(current.size(), SlotContent{"", 0});
    }

    for (std::size_t i = 0; i < current.size(); ++i) {
        const SlotContent& old = view.slots[i];
        const SlotContent& now = current[i];
        if (old == now) {
            continue;
        }
        const auto slot = static_cast<uint8_t>(i);
        out.push_back(ServerInventorySlot{slot, now.count, now.content_id});

        // Obtención: la cantidad sube o aparece un contenido nuevo. Al primer
        // sync no hay obtención: el inventario inicial no es un evento.
        const bool gained = view.synced && (now.count > old.count ||
                                            (now.count > 0 && now.content_id != old.content_id));
        if (gained) {
            out.push_back(ServerObtained{now.content_id});
        }
    }
    view.slots = std::move(current);
}
