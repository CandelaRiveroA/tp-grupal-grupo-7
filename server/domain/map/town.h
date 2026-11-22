#ifndef TOWN_H
#define TOWN_H

#include <cstdint>
#include <memory>
#include <vector>

#include "../npc/merchant.h"
#include "scene.h"

class World;

class Town: public Scene {
private:
    std::vector<Merchant*> merchants;

    
    void place_default_merchants();
    
protected:
    void on_entity_removed(IEntity* entity) override;

public:
    explicit Town(uint16_t width = 16, uint16_t height = 12);
    ~Town() override = default;

    Merchant* get_merchant_at(uint16_t x, uint16_t y);
    Merchant* get_nearest_merchant(uint16_t x, uint16_t y) const;
    void place_merchant(std::unique_ptr<Merchant> merchant);
    void on_new_day(World& world) override;
};

#endif  // TOWN_H
