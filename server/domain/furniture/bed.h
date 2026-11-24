#ifndef BED_H
#define BED_H

#include "furniture.h"

// Cama: pisarla pide dormir (ver World::record_sleep_request). Es el único
// mueble caminable, porque el jugador tiene que superponerse a su celda.
class Bed: public Furniture {
public:
    Bed(uint16_t x, uint16_t y);
    ~Bed() override = default;
    Bed(const Bed&) = delete;
    Bed& operator=(const Bed&) = delete;
    Bed(Bed&&) = default;
    Bed& operator=(Bed&&) = default;

    bool is_walkable() const override;
    void interact(Player& p, ToolType tool, World& w) override;

protected:
    void on_use(Player& p, World& w) override;
};

#endif  // BED_H
