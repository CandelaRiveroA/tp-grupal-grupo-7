#include "commands.h"

namespace commands {

void apply(World& world, uint8_t player_id, const protocol::DonateCommunityMessage& message) {
    world.donate_to_community_center(player_id, static_cast<CommunitySet>(message.set_id));
}

void apply(World& world, uint8_t player_id, const protocol::SelectSlotMessage& message) {
    world.select_slot(player_id, message.slot);
}

void apply(World& world, uint8_t player_id, const protocol::TakeMessage& message) {
    world.set_player_facing(player_id, message.direction);
    // La selección del hotbar es local: sin esto el server tendría el
    // casillero viejo y no sabría qué tiene equipado (el regalo lo pide).
    world.select_slot(player_id, message.slot);
    world.take_item(player_id);
}

void apply(World& world, uint8_t player_id, const protocol::DiscardMessage& m) {
    world.discard_item(player_id, m.slot, m.amount);
}

void apply(World& world, uint8_t player_id, const protocol::OpenContainerMessage& m) {
    world.open_container(player_id, m.x, m.y);
}

void apply(World& world, uint8_t player_id, const protocol::CloseContainerMessage&) {
    world.close_container(player_id);
}

void apply(World& world, uint8_t player_id, const protocol::ContainerTransferMessage& m) {
    world.transfer_container_item(player_id, m.x, m.y,
            m.direction == protocol::ContainerDirection::ToContainer, m.slot, m.amount);
}

void apply(World& world, uint8_t player_id, const protocol::PlaceFurnitureMessage& m) {
    world.place_chest(player_id, m.x, m.y, m.slot, m.facing);
}

void apply(World& world, uint8_t player_id, const protocol::GrabFurnitureMessage& m) {
    world.grab_chest(player_id, m.x, m.y);
}

}  // namespace commands
