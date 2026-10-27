#include "mineral.h"

Mineral::Mineral(uint16_t x, uint16_t y, uint8_t hits_remaining) : IEntity(x, y), hits_remaining(hits_remaining) {}

bool Mineral::is_walkable() const {
    return false;
}

void Mineral::interact(Player& p, ToolType tool, World& w) {
    if (tool == ToolType::PICKAXE) {
        if (this->hits_remaining > 0) {
            this->hits_remaining--;
        }
        //TODO: Separar responsabilidad de minar el mineral en un método
        // o la validacion en otra parte
        if (this->hits_remaining == 0) {
            this->mine_mineral(p, w);
        }
    }
}