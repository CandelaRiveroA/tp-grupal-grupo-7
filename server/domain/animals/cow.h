#ifndef COW_H
#define COW_H

#include "animal.h"

class Cow : public Animal {
private:
    bool is_ready_to_milk;

public:
    Cow(uint16_t x, uint16_t y);
    ~Cow() override = default;

    void update() override;
    void produce(World& w) override;
    
    void interact(Player& p, ToolType tool, World& w) override;
};

#endif // COW_H
