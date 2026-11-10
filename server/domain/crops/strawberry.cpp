#include "strawberry.h"
#include "../map/world.h"
#include "../inventory/product.h"
#include "../entities/dropped_item.h"
#include <memory>

Strawberry::Strawberry(uint16_t x, uint16_t y) 
    : Crop(x, y, 4, 2, ItemID::STRAWBERRY, 25) {}

void Strawberry::harvest(Player& p, World& w) {
    auto product = std::make_unique<Product>(this->product_id, this->base_price, this->current_quality);
    
    // 2. Intentamos dársela al jugador
    bool success = p.pick_up_item(std::move(product));
    
    // 3. Si se guardó en la mochila, aplicamos la regla del rebrote
    if (success) {
        w.record_pickup(p.get_id(), this->product_id);
        this->is_ready_to_harvest = false;
        // La frutilla tarda 4 días la primera vez, pero el rebrote tarda 2 días.
        // Restándole 2 a los días requeridos, arrancará desde la mitad de su ciclo.
        this->days_alive = this->required_days_to_harvest - 2; 
        this->current_quality = Quality::BASE; // Reseteamos la calidad para el nuevo ciclo
    }
}
