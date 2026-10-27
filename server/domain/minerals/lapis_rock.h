#ifndef LAPIS_ROCK_H
#define LAPIS_ROCK_H

#include "mineral.h"

class LapisRock : public Mineral {
public:
    LapisRock(uint16_t x, uint16_t y);
    ~LapisRock() override = default;

protected:
    void mine_mineral(Player& p, World& w) override;
};

#endif // LAPIS_ROCK_H
