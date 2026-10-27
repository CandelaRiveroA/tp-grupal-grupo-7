#ifndef PLAYER_H
#define PLAYER_H

#include <cstdint>
#include <utility>
#include <memory>
#include "i_entity.h"
#include "../inventory/inventory.h"
#include "../inventory/item.h"
#include "../../../common/common_constants.h"

class Player : public IEntity {
private:
    uint32_t player_id;
    Direction facing;
    uint8_t selected_slot;
    Inventory backpack;

public:
    Player(uint32_t id, uint16_t x, uint16_t y);
    ~Player() override = default;

    Player(const Player&) = delete;
    Player& operator=(const Player&) = delete;
    Player(Player&&) = default;
    Player& operator=(Player&&) = default;

    // Observadores espaciales y de estado
    // Necesario para el DTO
    uint32_t get_id() const;
    Direction get_facing() const;
    const Inventory& get_inventory() const; 

    // Movimiento y orientación
    void set_facing(Direction dir);
    std::pair<uint16_t, uint16_t> get_target_tile() const;

    // Interacción con herramientas e inventario
    void select_slot(uint8_t slot_idx);
    ToolType get_equipped_tool() const;
    bool pick_up_item(std::unique_ptr<Item> item);

    // Contrato IEntity
    bool is_walkable() const override;
    void interact(Player& p, ToolType tool, World& w) override;
};

#endif // PLAYER_H