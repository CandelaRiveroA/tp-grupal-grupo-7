// TODO: Recordad que el oro solo aparace después del nivel 5 de la mazmorra
#ifndef GOLD_ROCK_H
#define GOLD_ROCK_H

#include "mineral.h"

class GoldRock : public Mineral {
public:
    GoldRock(uint16_t x, uint16_t y);
    ~GoldRock() override = default;

protected:
    void mine_mineral(Player& p, World& w) override;
};

#endif // GOLD_ROCK_H
