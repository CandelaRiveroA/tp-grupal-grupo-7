#ifndef FURNITURE_H
#define FURNITURE_H

#include <cstdint>

#include "../entities/i_entity.h"
#include "../inventory/item_types.h"

// Mueble (decoración) del mundo: se coloca en una celda, se destruye de un
// solo golpe de pico y deja un DecorationItem en el piso. La subclase
// decide qué pasa al usarlo con la mano (la cama pide dormir).
class Furniture: public IEntity {
protected:
    ItemID drop_id;
    uint16_t model_id;

    // Base: sin efecto. Solo se llama con la mano y parado en la celda.
    virtual void on_use(Player& p, World& w);

public:
    Furniture(uint16_t x, uint16_t y, ItemID drop_id, uint16_t model_id);
    ~Furniture() override = default;
    Furniture(const Furniture&) = delete;
    Furniture& operator=(const Furniture&) = delete;
    Furniture(Furniture&&) = default;
    Furniture& operator=(Furniture&&) = default;

    bool is_walkable() const override;
    void interact(Player& p, ToolType tool, World& w) override;
    std::optional<CellSnapshot> snapshot() const override;
};

#endif  // FURNITURE_H
