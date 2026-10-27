#include "game/MockGameClient.h"

#include <algorithm>
#include <cstddef>

#include "world/CreatureCatalog.h"
#include "world/Grid.h"

namespace game {

MockGameClient::MockGameClient(int gridColumns, int gridRows, int startX,
			       int startY, int toolUseDurationTicks)
	: gridColumns_(gridColumns)
	, gridRows_(gridRows)
	, playerCellX_(startX)
	, playerCellY_(startY)
	, playerFacing_(protocol::Direction::Front)
	, playerMoving_(false)
	, toolUseDurationTicks_(toolUseDurationTicks > 0
					? toolUseDurationTicks
					: 1) {
	placeMinerals();
	rebuildInventory();
	rebuildView();
}

void MockGameClient::placeMinerals() {
	// Piedras de prueba alrededor del jugador. El servidor real decidirá
	// qué y dónde; esto solo simula un mundo con minerales variados.
	const int cx = playerCellX_;
	const int cy = playerCellY_;
	const std::vector<MineralSpot> spots = {
		{world::ResourceKind::Silver, cx + 1, cy, 0},
		{world::ResourceKind::Coal, cx + 2, cy, 0},
		{world::ResourceKind::Lapis, cx + 1, cy + 1, 0},
		{world::ResourceKind::Gold, cx + 2, cy + 1, 0},
		{world::ResourceKind::Coal, cx - 1, cy, 0},
		{world::ResourceKind::Silver, cx, cy + 2, 0},
		{world::ResourceKind::Gold, cx - 1, cy + 2, 0},
		{world::ResourceKind::Lapis, cx + 3, cy, 0},
	};
	for (const MineralSpot& spot : spots) {
		if (spot.cellX < 0 || spot.cellX >= gridColumns_ ||
		    spot.cellY < 0 || spot.cellY >= gridRows_) {
			continue;
		}
		minerals_.push_back(spot);
	}
}

void MockGameClient::sendCommand(const protocol::Command& command) {
	switch (command.move) {
	case protocol::Move::Walk:
		toolUseRemaining_ = 0;
		walkTo(command.x, command.y, command.direction);
		break;
	case protocol::Move::Pick:
		playerMoving_ = false;
		playerFacing_ = command.direction;
		toolUseRemaining_ = toolUseDurationTicks_;
		// El daño se confirma al COMPLETAR la ventana (fin de la
		// animación), no al apretar, como hará el servidor.
		pendingMineral_ = mineralAt(command.x, command.y);
		break;
	}
	rebuildView();
}

int MockGameClient::mineralAt(int cellX, int cellY) const {
	for (std::size_t i = 0; i < minerals_.size(); ++i) {
		if (minerals_[i].cellX == cellX &&
		    minerals_[i].cellY == cellY) {
			return static_cast<int>(i);
		}
	}
	return -1;
}

void MockGameClient::walkTo(int targetX, int targetY,
			    protocol::Direction direction) {
	const int clampedX = std::clamp(targetX, 0, gridColumns_ - 1);
	const int clampedY = std::clamp(targetY, 0, gridRows_ - 1);

	playerMoving_ =
		(clampedX != playerCellX_ || clampedY != playerCellY_);
	playerCellX_ = clampedX;
	playerCellY_ = clampedY;
	playerFacing_ = direction;
}

void MockGameClient::advanceToolUse() const {
	if (toolUseRemaining_ <= 0) {
		return;
	}
	--toolUseRemaining_;
	// Al completar el golpe (0) se confirma el daño al mineral apuntado.
	if (toolUseRemaining_ == 0 && pendingMineral_ >= 0 &&
	    pendingMineral_ < static_cast<int>(minerals_.size())) {
		MineralSpot& spot =
			minerals_[static_cast<std::size_t>(pendingMineral_)];
		if (spot.hits < 3) {
			++spot.hits;
			// Al agotar la roca, el evento de obtención se programa
			// con retardo: el discover suena DESPUÉS del crack.
			if (spot.hits == 3) {
				obtainedCountdown_ = kObtainedDelayTicks;
				obtainedKind_ = spot.kind;
			}
		}
		pendingMineral_ = -1;
	}
}

void MockGameClient::advanceObtained() const {
	view_.obtained.clear();
	if (obtainedCountdown_ > 0) {
		--obtainedCountdown_;
		if (obtainedCountdown_ == 0) {
			view_.obtained.push_back(
				ViewResourceObtained{obtainedKind_});
		}
	}
}

void MockGameClient::rebuildView() const {
	view_.entities.clear();
	view_.grid = world::GridSpan{gridColumns_, gridRows_};

	ViewEntity player;
	player.kind = world::CreatureKind::Player;
	player.cellX = playerCellX_;
	player.cellY = playerCellY_;
	player.facing = playerFacing_;
	player.moving = playerMoving_;
	if (toolUseRemaining_ > 0) {
		player.action = world::ActionKind::UseTool;
		player.activeTool = world::ItemKind::Pickaxe;
	} else {
		player.action = playerMoving_ ? world::ActionKind::Walk
					      : world::ActionKind::Idle;
	}
	view_.entities.push_back(player);

	rebuildResources();
}

void MockGameClient::rebuildResources() const {
	view_.resources.clear();
	// hits: 0 intacto, 1 cracked, 2 broken, 3+ agotado (sale de la vista).
	for (const MineralSpot& spot : minerals_) {
		world::ResourceState state;
		if (spot.hits == 0) {
			state = world::ResourceState::Intact;
		} else if (spot.hits == 1) {
			state = world::ResourceState::Cracked;
		} else if (spot.hits == 2) {
			state = world::ResourceState::Broken;
		} else {
			continue;  // agotado
		}
		ViewResource resource;
		resource.kind = spot.kind;
		resource.cellX = spot.cellX;
		resource.cellY = spot.cellY;
		resource.state = state;
		view_.resources.push_back(resource);
	}
}

const LocalWorldView& MockGameClient::worldState() const {
	advanceToolUse();
	rebuildView();
	advanceObtained();
	return view_;
}

void MockGameClient::rebuildInventory() {
	view_.inventory.clear();
	view_.inventory.push_back({0, world::ItemKind::Pickaxe});
	view_.inventory.push_back({1, world::ItemKind::Shovel});
	view_.inventory.push_back({2, world::ItemKind::WateringCan});
	view_.inventory.push_back({3, world::ItemKind::Sword});
}

}
