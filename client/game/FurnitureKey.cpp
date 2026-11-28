#include "game/FurnitureKey.h"

namespace game {

std::string FurnitureKey::of(const ViewFurniture& furniture) const {
	return furniture.contentId + "@" + std::to_string(furniture.cellX) + "," +
	       std::to_string(furniture.cellY);
}

}
