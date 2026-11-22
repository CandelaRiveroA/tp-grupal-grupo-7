#ifndef TOWN_H
#define TOWN_H

#include <cstdint>
#include <memory>
#include <vector>

#include "../entities/i_entity.h"
#include "../map/cell.h"
#include "../npc/merchant.h"

class Town {
private:
    uint16_t width;
    uint16_t height;
    std::vector<std::vector<Cell>> grid;
    // Punteros no-dueños a los comerciantes del pueblo (los dueños son las celdas de grid)
    std::vector<Merchant*> merchants;

    void place_default_merchants();

public:
    explicit Town(uint16_t width = 16, uint16_t height = 12);
    ~Town() = default;

    Town(const Town&) = delete;
    Town& operator=(const Town&) = delete;
    Town(Town&&) = default;
    Town& operator=(Town&&) = default;

    // Operaciones genéricas de tablero (alineadas con Cell::entities y World)
    uint16_t get_width() const;
    uint16_t get_height() const;
    bool in_bounds(uint16_t x, uint16_t y) const;
    bool is_walkable(uint16_t x, uint16_t y) const;

    void set_wall(uint16_t x, uint16_t y, bool wall);
    void set_entity_at(uint16_t x, uint16_t y, std::unique_ptr<IEntity> entity);
    void remove_entity(IEntity* target_entity);
    std::vector<IEntity*> get_entities_at(uint16_t x, uint16_t y) const;

    // Lógica exclusiva del Pueblo
    Merchant* get_merchant_at(uint16_t x, uint16_t y);
    void on_new_day();
};

#endif  // TOWN_H