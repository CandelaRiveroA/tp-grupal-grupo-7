#ifndef TOWN_H
#define TOWN_H

#include <cstdint>
#include <memory>
#include <vector>

#include "../npc/merchant.h"
#include "scene.h"
#include "world_model/Marker.h"

class World;

class Town: public Scene {
public:
    explicit Town(uint16_t width = 16, uint16_t height = 12);
    ~Town() override = default;

    void place_default_merchants();
    static std::unique_ptr<Merchant> make_merchant(worldmodel::MarkerKind kind, uint16_t x, uint16_t y);
    void on_new_day(World& world) override;
};

#endif  // TOWN_H
