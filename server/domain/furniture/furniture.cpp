#include "furniture.h"

#include <memory>
#include <utility>

#include "../entities/dropped_item.h"
#include "../inventory/decoration_item.h"
#include "../map/world.h"

Furniture::Furniture(uint16_t x, uint16_t y, ItemID drop_id, uint16_t model_id):
        IEntity(x, y), drop_id(drop_id), model_id(model_id) {}

bool Furniture::is_walkable() const { return false; }

std::optional<CellSnapshot> Furniture::snapshot() const {
    return CellSnapshot{CellSnapshot::Kind::Furniture, this->drop_id, 0, 1};
}

void Furniture::on_use(Player& /*p*/, World& /*w*/) {}

void Furniture::interact(Player& p, ToolType tool, World& w) {
    if (tool == ToolType::PICKAXE) {
        const uint16_t x = this->pos_x;
        const uint16_t y = this->pos_y;
        const ItemID drop = this->drop_id;
        const uint16_t model = this->model_id;

        // Desde acá this queda destruido: solo se usan los valores copiados.
        w.remove_entity(this);
        auto decoration = std::make_unique<DecorationItem>(drop, model);
        auto dropped = std::make_unique<DroppedItem>(std::move(decoration), x, y);

        w.set_entity_at(x, y, std::move(dropped));
        return;
    }

    // La mano solo sirve parado encima: así se pide dormir sin volver a
    // caminar sobre la cama.
    if (tool == ToolType::HAND && p.get_x() == this->pos_x && p.get_y() == this->pos_y) {
        on_use(p, w);
    }
}
