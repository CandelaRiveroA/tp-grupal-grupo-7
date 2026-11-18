#include "crop.h"
#include "../map/world.h"
#include "../entities/player.h"
#include <random>
#include <algorithm>

#define kHarvestStage 6

Crop::Crop(uint16_t x, uint16_t y, uint16_t req_days, uint16_t req_water, ItemID p_id, uint16_t b_price)
    : IEntity(x, y), days_alive(0), required_days_to_harvest(req_days), 
        is_ready_to_harvest(false), required_water_per_day(req_water),
        times_watered_today(0), days_watered_successfully(0), 
        product_id(p_id), base_price(b_price), current_quality(Quality::BASE) {}

void Crop::degrade_quality_over_time() {
    if (this->current_quality > Quality::BASE) {
        int val = static_cast<int>(this->current_quality);
        this->current_quality = static_cast<Quality>(val - 1);
    }
}

int Crop::calculate_watering_score() const {
    if (this->required_days_to_harvest == 0) return 0;

    float ratio = static_cast<float>(this->days_watered_successfully) / this->required_days_to_harvest;
    return static_cast<int>(std::min(ratio, 1.0f) * 70.0f);
}

int Crop::generate_luck_score() const {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> distrib(0, 40);
    
    return distrib(gen);
}

Quality Crop::map_score_to_quality(int total_score) const {
    if (total_score >= 105) return Quality::PERFECT;
    if (total_score >= 90)  return Quality::INCREDIBLE;
    if (total_score >= 70)  return Quality::EXCELLENT;
    if (total_score >= 40)  return Quality::GOOD;
    
    return Quality::BASE;
}

Quality Crop::calculate_quality() const {
    int base_score = this->calculate_watering_score();
    int luck_score = this->generate_luck_score();
    
    return this->map_score_to_quality(base_score + luck_score);
}

bool Crop::is_walkable() const {
    return true; 
}

bool Crop::blocks_placement() const {
    return true; 
}

void Crop::update(World& /*w*/) {
    // Si ya está lista, ya no crece ni consume agua, solo se pudre esperando
    if (this->is_ready_to_harvest) {
        this->degrade_quality_over_time();
        return; 
    }

    this->days_alive++;

    // Evalúa si se cumplió la cuota exacta o superior de agua hoy
    if (this->times_watered_today >= this->required_water_per_day) {
        this->days_watered_successfully++;
    }
    
    // El agua se evapora/consume, reseteamos para el nuevo día
    this->times_watered_today = 0;

    if (this->days_alive == this->required_days_to_harvest) {
        this->is_ready_to_harvest = true;
        // La calidad se decide al llegar la cosecha (riego + suerte)
        this->current_quality = this->calculate_quality();
    } else if (this->days_alive > this->required_days_to_harvest) {
        // Si el jugador no cosechó a tiempo, el cultivo se pudre y baja de calidad.
        this->degrade_quality_over_time();
    }
}

bool Crop::pickup_on_take() const { return this->is_ready_to_harvest; }

std::optional<CropSnapshot> Crop::crop_snapshot() const {
    const bool ready = this->is_ready_to_harvest;
    const uint16_t days = std::max<uint16_t>(this->required_days_to_harvest, 1);
    const auto growing = static_cast<uint8_t>(std::min<uint16_t>(5, this->days_alive * 6 / days));
    return CropSnapshot{this->product_id,
                        static_cast<uint8_t>(ready ? kHarvestStage : growing),
                        !ready && this->times_watered_today < this->required_water_per_day, ready,
                        this->current_quality};
}

void Crop::interact(Player& p, ToolType tool, World& w) {
    if (tool == ToolType::WATERING_CAN) {
        // Para que el jugador no pase el limite de la cantidad de regadas por dia
        // Podria causar un error donde siempre saca maxima calidad si maxea el primer dia
        if (this->times_watered_today < this->required_water_per_day) {
            this->times_watered_today++;
        }
    } 
    else if (tool == ToolType::HAND && this->is_ready_to_harvest) {
        this->harvest(p, w);
    }
}