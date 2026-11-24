#ifndef PORTAL_H
#define PORTAL_H

#include <cstdint>

#include "../entities/i_entity.h"
#include "scene.h"

class Portal: public IEntity {
private:
    SceneId target_scene;
    uint16_t target_x;
    uint16_t target_y;

public:
    Portal(uint16_t x, uint16_t y, SceneId target_scene, uint16_t target_x, uint16_t target_y);
    ~Portal() override = default;

    bool is_walkable() const override;
    void interact(Player& p, ToolType tool, World& w) override;
    void update(World& w) override;
};

#endif  // PORTAL_H
