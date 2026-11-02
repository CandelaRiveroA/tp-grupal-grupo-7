#include "game/FloorItems.h"

#include <cstdlib>

#include "game/GameConstants.h"

namespace game {

bool FloorItems::tryDrop(const world::ItemKind kind, const int cellX,
			 const int cellY) {
	if (static_cast<int>(items_.size()) >= kMaxGroundItems) {
		return false;
	}
	for (const FloorItem& item : items_) {
		if (item.cellX == cellX && item.cellY == cellY) {
			return false;
		}
	}
	items_.push_back(FloorItem{kind, cellX, cellY});
	return true;
}

const std::vector<FloorItem>& FloorItems::items() const { return items_; }

std::optional<InteractionTarget> FloorItems::findTargetNear(
	const int cellX, const int cellY) const {
	for (std::size_t i = 0; i < items_.size(); ++i) {
		const FloorItem& item = items_[i];
		// Cerca = las celdas se tocan (adyacentes o la misma).
		if (std::abs(item.cellX - cellX) > 1 ||
		    std::abs(item.cellY - cellY) > 1) {
			continue;
		}
		InteractionTarget target;
		target.action = world::ActionKind::Pickup;
		target.cycles = kPickupCycles;
		const std::string* contentId = contentIds_.find(item.kind);
		target.reward = contentId != nullptr ? *contentId : "";
		target.held = item.kind;
		target.targetId = static_cast<int>(i);
		target.cellX = item.cellX;
		target.cellY = item.cellY;
		return target;
	}
	return std::nullopt;
}

void FloorItems::onInteractionStarted(const InteractionTarget& target) {
	// El ítem queda en las manos: sale del piso.
	if (target.targetId < 0 ||
	    target.targetId >= static_cast<int>(items_.size())) {
		return;
	}
	items_.erase(items_.begin() + target.targetId);
}

void FloorItems::onInteractionFinished(const InteractionTarget&) {
	// Nada que hacer: el ítem ya salió del piso al empezar.
}

}  // namespace game
