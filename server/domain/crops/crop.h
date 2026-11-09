#ifndef CROP_H
#define CROP_H

#include "../entities/i_entity.h"
#include "../inventory/item_types.h"
#include <cstdint>

class World;
class Player;

//TODO: Todos los precios de los cultivos deben ser cargados desde
// el archivo toml para su modificacion de precio base

class Crop : public IEntity {
protected:
    uint16_t days_alive;
    uint16_t required_days_to_harvest;
    bool is_ready_to_harvest;
    
    // Contadores para el cálculo de calidad
    uint16_t required_water_per_day;
    uint16_t times_watered_today;
    uint16_t days_watered_successfully; 
    
    ItemID product_id;
    uint16_t base_price;
    Quality current_quality;

    // Calculos de calidad
    Quality calculate_quality() const;
    int calculate_watering_score() const;
    int generate_luck_score() const;
    Quality map_score_to_quality(int score) const;
    void degrade_quality_over_time();

public:
    Crop(uint16_t x, uint16_t y, uint16_t req_days, uint16_t req_water, ItemID p_id, uint16_t b_price);
    virtual ~Crop() = default;

    Crop(const Crop&) = delete;
    Crop& operator=(const Crop&) = delete;
    Crop(Crop&&) = default;
    Crop& operator=(Crop&&) = default;

    void update(World& w) override; 
    void interact(Player& p, ToolType tool, World& w) override;

    virtual void harvest(World& w) = 0;
};

#endif // CROP_H
