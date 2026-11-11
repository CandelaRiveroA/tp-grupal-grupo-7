#ifndef COW_H
#define COW_H

#include <string>

#include "animal.h"

class Cow: public Animal {
private:
    bool is_ready_to_milk;

public:
    Cow(uint16_t x, uint16_t y);
    ~Cow() override = default;

    bool is_walkable() const override;
    std::string kind_name() const override;

    std::optional<AnimalSnapshot> animal_snapshot() const override;

    void update(World& w) override;
    void produce(World& w) override;

    void interact(Player& p, ToolType tool, World& w) override;

    // La mano solo sirve si la vaca está con leche lista; el mundo no consulta
    // ese estado, le pide a la vaca que actúe.
    bool try_hand_action(Player& p, World& w) override;
};

#endif  // COW_H
