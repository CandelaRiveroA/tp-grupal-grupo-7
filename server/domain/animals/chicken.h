#ifndef CHICKEN_H
#define CHICKEN_H

#include <string>

#include "animal.h"

class Chicken: public Animal {
public:
    Chicken(uint16_t x, uint16_t y);
    ~Chicken() override = default;

    bool is_walkable() const override;
    std::string kind_name() const override;

    void update(World& w) override;
    void produce(World& w) override;
    void interact(Player& p, ToolType tool, World& w) override;
};

#endif  // CHICKEN_H
