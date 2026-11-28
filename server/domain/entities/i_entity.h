#ifndef IENTITY_H
#define IENTITY_H

#include <cstdint>
#include <optional>
#include <string>

#include "../../../common/common_constants.h"
#include "../inventory/item_types.h"

class World;
class Player;
class Inventory;

// Descripción neutral de cómo se ve una celda, para que el mundo la muestre
// sin conocer el tipo concreto de la entidad.
struct CellSnapshot {
    enum class Kind { Resource, Dropped, Furniture };

    Kind kind;
    ItemID item;
    uint8_t hits_taken;
    uint8_t hits_to_destroy;
};

// Estado visible de un animal, para que el mundo lo envíe sin conocer su tipo.
struct AnimalSnapshot {
    uint8_t id;
    std::string kind;
    bool ready_to_milk;
    Direction facing;
};

// Estado visible de un enemigo: vida, hacia dónde mira y la acción que anima el cliente.
struct EnemySnapshot {
    uint16_t id;
    std::string kind;
    Direction facing;
    uint8_t health;
    EnemyAction action;
};

// Estado visible de un proyectil (llama del skull), para que el mundo lo
// envíe sin conocer su tipo. No lleva vida ni acción: solo dónde va y hacia
// dónde mira.
struct ProjectileSnapshot {
    uint16_t id;
    std::string kind;
    uint16_t x;
    uint16_t y;
    Direction direction;
};

// Estado visible de un cultivo: el producto que da y la etapa (0 semilla,
// 1..5 crecimiento, 6 cosecha), si pide agua, si ya se puede cosechar y la
// calidad de esa cosecha (BASE..PERFECT).
struct CropSnapshot {
    ItemID product;
    uint8_t stage;
    bool needs_water;
    bool ready;
    Quality quality;
};

class IEntity {
protected:
    uint16_t pos_x;
    uint16_t pos_y;

public:
    IEntity(uint16_t x, uint16_t y);
    virtual ~IEntity() = default;

    uint16_t get_x() const;
    uint16_t get_y() const;

    void set_position(uint16_t x, uint16_t y);

    // Determina si otra entidad (como el jugador o enemigos) puede caminar SOBRE esta entidad.
    // Por defecto, asumimos que la mayoria bloquean el paso.
    // DroppedItem podría sobrescribir esto para retornar 'true' si quieres que el
    // jugador pueda pararse en la misma casilla que un ítem tirado antes de recogerlo.
    virtual bool is_walkable() const = 0;
    virtual bool blocks_placement() const;
    virtual Inventory* container_inventory();
    virtual const Inventory* container_inventory() const;
    virtual uint8_t furniture_facing() const;

    virtual void interact(Player& p, ToolType tool, World& w) = 0;

    // true = se levanta con Take y eso dispara la animación de recogida.
    virtual bool pickup_on_take() const;

    // Nullopt si la entidad no se muestra como contenido de celda.
    virtual std::optional<CellSnapshot> snapshot() const;

    // Nullopt si la entidad no es un animal.
    virtual std::optional<AnimalSnapshot> animal_snapshot() const;

    // Nullopt si la entidad no es un enemigo.
    virtual std::optional<EnemySnapshot> enemy_snapshot() const;

    // Nullopt si la entidad no es un cultivo.
    virtual std::optional<CropSnapshot> crop_snapshot() const;

    // Método para que las entidades se actualicen en cada frame del GameLoop.
    // Por defecto no hace nada (útil para minerales).
    // Los NPC o cultivos lo sobreescribirán.
    virtual void update(World& w);
};

#endif  // IENTITY_H
