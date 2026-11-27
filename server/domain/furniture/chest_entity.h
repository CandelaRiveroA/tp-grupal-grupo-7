#ifndef CHEST_ENTITY_H
#define CHEST_ENTITY_H

#include "furniture.h"
#include "../inventory/inventory.h"

#define CHEST_INVENTORY_SIZE 30

// Cofre en el mapa. Hereda de Furniture para comportarse como un mueble,
// pero contiene por composición un Inventory de 30 espacios.
class ChestEntity : public Furniture {
private:
    Inventory storage; 

protected:
    // Se ejecuta cuando el jugador interactúa con las manos vacías
    void on_use(Player& p, World& w) override;

public:
    // El cofre se inicializa siempre con 30 casilleros.
    ChestEntity(uint16_t x, uint16_t y, ItemID drop_id, uint16_t model_id);
    ~ChestEntity() override = default;

    ChestEntity(const ChestEntity&) = delete;
    ChestEntity& operator=(const ChestEntity&) = delete;
    ChestEntity(ChestEntity&&) = default;
    ChestEntity& operator=(ChestEntity&&) = default;

    void interact(Player& p, ToolType tool, World& w) override;

    // Acceso al inventario para que el GameMonitor pueda transferir ítems
    Inventory& get_inventory();
    const Inventory& get_inventory() const;

    bool is_empty() const;
};

#endif // CHEST_ENTITY_H
