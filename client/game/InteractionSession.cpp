#include "game/InteractionSession.h"

#include <cstdlib>

namespace game {

void InteractionSession::start(const InteractionTarget& target,
			       const int playerCellX, const int playerCellY,
			       const world::CreatureData& creature) {
	target_ = target;
	facing_ = facingToward(playerCellX, playerCellY, target.cellX,
			      target.cellY);
	facingOverride_ = true;
	// N ciclos completos de la animación, con los mismos datos que usa
	// el avance de cuadros. Nunca 0: la ventana mínima es 1 tick.
	const int window =
		target.cycles * creature.actionCycleTicks(target.action);
	ticksRemaining_ = window > 0 ? window : 1;
}

bool InteractionSession::active() const { return ticksRemaining_ > 0; }

bool InteractionSession::advance() {
	if (ticksRemaining_ <= 0) {
		return false;
	}
	--ticksRemaining_;
	if (ticksRemaining_ > 0) {
		return false;
	}
	// Cierre: se entrega la recompensa.
	rewardPending_ = true;
	recordGain(target_.reward);
	return true;
}

const InteractionTarget& InteractionSession::target() const {
	return target_;
}

void InteractionSession::mergePlayer(ViewEntity& player) {
	if (!facingOverride_) {
		return;
	}
	if (active()) {
		player.action = target_.action;
		player.facing = facing_;
		player.activeTool = target_.held;
		return;
	}
	if (player.action == world::ActionKind::Idle) {
		player.facing = facing_;
		return;
	}
	// El jugador hizo otra cosa: la fuente recupera el facing.
	facingOverride_ = false;
}

void InteractionSession::mergeRewards(LocalWorldView& view) {
	if (rewardPending_) {
		view.obtained.push_back(ViewObtained{target_.reward});
		rewardPending_ = false;
	}
	// En orden de obtención y cada uno en SU slot (el que le tocó la
	// primera vez): nada se reordena ni le roba el cuadrado a otro.
	for (InventoryGain& gain : inventoryGains_) {
		if (gain.slotIndex >= 0 &&
		    !view.inventorySlotFree(gain.slotIndex)) {
			// Otra fuente ocupó su lugar: se reubica (único
			// caso donde una entrega cambia de slot).
			gain.slotIndex = -1;
		}
		if (gain.slotIndex < 0) {
			gain.slotIndex = view.addInventoryContent(gain.contentId);
		}
		view.placeInventoryContent(gain.slotIndex, gain.contentId,
					   gain.count);
	}
}

void InteractionSession::recordGain(const std::string& contentId) {
	for (InventoryGain& gain : inventoryGains_) {
		if (gain.contentId == contentId) {
			++gain.count;
			return;
		}
	}
	inventoryGains_.push_back(InventoryGain{contentId, -1, 1});
}

world::Direction InteractionSession::facingToward(const int fromX,
						  const int fromY,
						  const int toX,
						  const int toY) const {
	const int dx = toX - fromX;
	const int dy = toY - fromY;
	if (std::abs(dx) >= std::abs(dy)) {
		return dx >= 0 ? world::Direction::Right : world::Direction::Left;
	}
	return dy >= 0 ? world::Direction::Front : world::Direction::Back;
}

}  // namespace game
