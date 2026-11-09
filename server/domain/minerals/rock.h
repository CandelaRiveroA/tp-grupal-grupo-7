#ifndef ROCK_H
#define ROCK_H

#include "mineral.h"

// Roca de relleno de la sala: un solo golpe y no deja nada al romperse.
class Rock: public Mineral {
public:
    Rock(uint16_t x, uint16_t y);
    ~Rock() override = default;
    ItemID get_drop_id() const override;

protected:
    void mine_mineral(World& w) override;
};

#endif  // ROCK_H
