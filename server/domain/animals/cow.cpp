#include "cow.h"

#include <memory>
#include <utility>

#include "../entities/player.h"
#include "../inventory/product.h"
#include "../map/world.h"

Cow::Cow(uint16_t x, uint16_t y): Animal(x, y), is_ready_to_milk(false) {}

bool Cow::is_walkable() const { return false; }

std::string Cow::kind_name() const { return "cow"; }

std::optional<AnimalSnapshot> Cow::animal_snapshot() const {
    return AnimalSnapshot{get_id(), kind_name(), is_ready_to_milk, wander_dir};
}

void Cow::update(World& /*w*/) {
    // Se ejecuta con cada día del juego
    this->days_passed++;
    // TODO: Sacar numeros mágicos a constantes o configuraciones externas
    if (this->days_passed >= 2) {
        // La vaca da leche cada 2 días
        this->is_ready_to_milk = true;
    }
}

void Cow::produce(World& /*w*/) {}

void Cow::interact(Player& p, ToolType tool, World& w) {
    if (this->is_ready_to_milk && tool == ToolType::HAND) {

        auto milk = std::make_unique<Product>(ItemID::MILK, 30, Quality::BASE);

        // El jugador intenta guardarla en el inventario
        bool success = p.pick_up_item(std::move(milk));

        if (success) {
            this->is_ready_to_milk = false;
            this->days_passed = 0;  // Una vez ordeñada, la vaca empieza a contar de nuevo los días
                                    // para la próxima leche.
            w.record_pickup(p.get_id(), ItemID::MILK);
        }
        // Si no hubo éxito (inventario lleno), la leche se destruye sola
        // y la vaca sigue lista para la próxima vez que interactúen con ella.
    }
}

bool Cow::try_hand_action(Player& p, World& w) {
    if (!this->is_ready_to_milk) {
        return false;
    }
    this->interact(p, ToolType::HAND, w);
    return true;
}
