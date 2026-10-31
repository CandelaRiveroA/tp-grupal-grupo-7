#include "mineral.h"

#include "../map/world.h"

Mineral::Mineral(uint16_t x, uint16_t y, uint8_t hits_remaining):
        IEntity(x, y), hits_remaining(hits_remaining), max_hits(hits_remaining) {}

bool Mineral::is_walkable() const { return false; }

std::optional<CellSnapshot> Mineral::snapshot() const {
    return CellSnapshot{CellSnapshot::Kind::Resource, get_drop_id(),
                        static_cast<uint8_t>(max_hits - hits_remaining), max_hits};
}

void Mineral::interact(Player& /*p*/, ToolType tool, World& w) {
    if (tool == ToolType::PICKAXE) {
        if (this->hits_remaining > 0) {
            this->hits_remaining--;
            w.record_resource_hit(pos_x, pos_y, hits_remaining == 0);
        }
        // TODO: Separar responsabilidad de minar el mineral en un método
        //  o la validacion en otra parte
        if (this->hits_remaining == 0) {
            this->mine_mineral(w);
        }
    }
}
