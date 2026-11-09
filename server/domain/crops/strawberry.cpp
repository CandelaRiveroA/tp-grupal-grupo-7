#include "strawberry.h"
#include "../map/world.h"
#include "../inventory/product.h"
#include "../entities/dropped_item.h"
#include <memory>

Strawberry::Strawberry(uint16_t x, uint16_t y) 
    : Crop(x, y, 4, 2, ItemID::STRAWBERRY, 25) {}

void Strawberry::harvest(World& w) {
    // 1. Usamos la calidad ya trackeada y fijada internamente por el update
    auto product = std::make_unique<Product>(this->product_id, this->base_price, this->current_quality);
    
    // 2. Instanciamos el DroppedItem en el mapa (Mismo patrón que el huevo)[cite: 6]
    auto drop = std::make_unique<DroppedItem>(this->get_x(), this->get_y(), std::move(product));
    w.set_entity_at(this->get_x(), this->get_y(), std::move(drop));

    // 3. Rebrote (restamos 2 días) Se puede cosechar cada 2 dias
    this->is_ready_to_harvest = false;
    this->days_alive = this->required_days_to_harvest - 2; 
    
    // Vuelve a Quality::BASE por las dudas, se recalculará al llegar al final del ciclo
    this->current_quality = Quality::BASE;
}
