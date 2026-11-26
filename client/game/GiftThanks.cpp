#include "game/GiftThanks.h"

namespace game {

void GiftThanks::update(const std::vector<uint8_t>& acceptedPlayers, int durationTicks) {
    for (auto it = remaining_.begin(); it != remaining_.end();) {
        if (--it->second <= 0) {
            it = remaining_.erase(it);
        } else {
            ++it;
        }
    }
    if (durationTicks > 0) {
        for (const uint8_t id: acceptedPlayers) remaining_[id] = durationTicks;
    }
}

void GiftThanks::apply(LocalWorldView& view) const {
    for (ViewEntity& entity: view.entities) {
        if (entity.kind == world::CreatureKind::Player && remaining_.count(entity.id) > 0) {
            entity.indication = world::Indication::GiftThanks;
        }
    }
}

}  // namespace game
