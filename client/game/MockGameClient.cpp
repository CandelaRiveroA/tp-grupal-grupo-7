#include "game/MockGameClient.h"

#include <algorithm>

#include "world/CreatureCatalog.h"

namespace game {

MockGameClient::MockGameClient(int gridColumns, int gridRows, int startX,
			       int startY)
	: gridColumns_(gridColumns)
	, gridRows_(gridRows)
	, playerCellX_(startX)
	, playerCellY_(startY)
	, playerFacing_(protocol::Direction::Front)
	, playerMoving_(false) {
	rebuildInventory();
	rebuildView();
}

void MockGameClient::sendCommand(const protocol::Command& command) {
	switch (command.move) {
	case protocol::Move::Walk:
		walkTo(command.x, command.y, command.direction);
		break;
	case protocol::Move::Pick:
		playerMoving_ = false;
		playerFacing_ = command.direction;
		break;
	}
	rebuildView();
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

void MockGameClient::rebuildView() {
	view_.entities.clear();

	ViewEntity player;
	player.kind = world::CreatureKind::Player;
	player.cellX = playerCellX_;
	player.cellY = playerCellY_;
	player.facing = playerFacing_;
	player.moving = playerMoving_;
	player.action = playerMoving_ ? world::ActionKind::Walk
				      : world::ActionKind::Idle;
	view_.entities.push_back(player);
}

const LocalWorldView& MockGameClient::worldState() const { return view_; }

void MockGameClient::rebuildInventory() {
	view_.inventory.clear();
	view_.inventory.push_back({0, world::ItemKind::Shovel});
	view_.inventory.push_back({1, world::ItemKind::Pickaxe});
	view_.inventory.push_back({2, world::ItemKind::Sword});
	view_.inventory.push_back({3, world::ItemKind::WateringCan});
}

}
