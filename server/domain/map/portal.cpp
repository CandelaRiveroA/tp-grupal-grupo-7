#include "portal.h"

#include "../entities/player.h"
#include "world.h"

Portal::Portal(uint16_t x, uint16_t y, SceneId target_scene,
               uint16_t target_x, uint16_t target_y):
        IEntity(x, y),
        target_scene(target_scene),
        target_x(target_x),
        target_y(target_y) {}

bool Portal::is_walkable() const { return true; }

void Portal::interact(Player& p, ToolType tool, World& w) {
    if (tool != ToolType::HAND) {
        return;
    }
    w.transfer_player(p.get_id(), this->target_scene, this->target_x, this->target_y);
}

void Portal::update(World& /*w*/) {}
