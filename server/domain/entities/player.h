#ifndef PLAYER_H
#define PLAYER_H

#include <cstdint>
#include <memory>
#include <utility>

#include "../../../common/common_constants.h"
#include "../../../common/model/draft/CharacterAppearance.h"
#include "../inventory/inventory.h"
#include "../inventory/item.h"

#include "i_attackable.h"
#include "i_entity.h"

class Player: public IEntity, public IAttackable {
private:
    uint8_t player_id;
    Direction facing;
    // Personalizacion del personaje (indices de capa, sin validar): la manda
    // el cliente en el MatchRequest y vuelve en cada ServerStatus.
    CharacterAppearance appearance;
    uint8_t selected_slot;
    Inventory backpack;

    uint16_t gold;
    float energy;
    float max_energy;
    // Vida: hp actual y max_hp tope (kMaxHealth). No hay otro campo de vida.
    uint16_t hp;
    uint16_t max_hp;
    bool died_in_combat;
    bool fainted_by_timeout;

public:
    Player(uint8_t id, uint16_t x, uint16_t y);
    ~Player() override = default;

    Player(const Player&) = delete;
    Player& operator=(const Player&) = delete;
    Player(Player&&) = default;
    Player& operator=(Player&&) = default;

    // Observadores espaciales y de estado
    // Necesario para el DTO
    uint8_t get_id() const;
    Direction get_facing() const;
    const CharacterAppearance& get_appearance() const;
    const Inventory& get_inventory() const;
    uint16_t get_health() const;
    uint16_t get_max_health() const;
    void set_health(uint16_t hp) { this->hp = hp; }
    void set_energy(float e) { this->energy = e; }
    float get_energy() const;
    float get_max_energy() const;

    bool diedInCombat() const { return this->died_in_combat; }
    bool faintedByTimeout() const { return this->fainted_by_timeout; }
    void setDiedInCombat(bool v) { this->died_in_combat = v; }
    void setFaintedByTimeout(bool v) { this->fainted_by_timeout = v; }

    // true si la vida quedó por debajo del 20 % del tope (herido grave o
    // muerto): con eso World decide si queda un charco de sangre.
    bool is_gravely_wounded() const;

    // IAttackable: true = quedó sin vida (sin consecuencias por ahora).
    bool take_damage(uint8_t incoming) override;

    // Movimiento y orientación
    void set_facing(Direction dir);
    void set_appearance(const CharacterAppearance& new_appearance);
    std::pair<uint16_t, uint16_t> get_target_tile() const;
    void consume_equipped_item();
    const Item* peek_equipped_item() const;

    //Energia y salud
    void consume_energy(float amount);
    bool is_exhausted() const; //Caso debajo 10% energia
    bool is_energy_depleted() const; //Caso debajo 0% energia

    // Interacción con herramientas e inventario
    void select_slot(uint8_t slot_idx);
    ToolType get_equipped_tool() const;
    bool pick_up_item(std::unique_ptr<Item> item);

    // Contrato IEntity
    bool is_walkable() const override;
    void interact(Player& p, ToolType tool, World& w) override;
    void update(World& w) override;
};

#endif  // PLAYER_H
