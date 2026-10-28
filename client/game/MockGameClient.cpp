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
	case protocol::Move::Take:
		// Recoger el item del piso en la celda del jugador. El
		// servidor validará; acá el mock decide si hay algo y, si lo
		// hay, programa el evento de obtención (panel + sonido).
		playerMoving_ = false;
		playerFacing_ = command.direction;
		collectAt(command.x, command.y);
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
			// Al agotar la roca, el mineral CAE al piso como item
			// recogible. Ya no se obtiene automático: el jugador
			// debe pararse sobre la celda y mandar Take (Enter).
			if (spot.hits == 3) {
				dropped_.push_back(DroppedItem{
					spot.kind, spot.cellX, spot.cellY});
			}
		}
		pendingMineral_ = -1;
	}
}

void MockGameClient::advanceObtained() const {
	view_.obtained.clear();
	// El evento de obtención lo dispara un Take exitoso (no se deduce):
	// se emite por un solo tick y luego se limpia.
	if (obtainedPending_) {
		view_.obtained.push_back(ViewResourceObtained{obtainedKind_});
		obtainedPending_ = false;
	}
}

int MockGameClient::droppedAt(int cellX, int cellY) const {
	for (std::size_t i = 0; i < dropped_.size(); ++i) {
		if (dropped_[i].cellX == cellX && dropped_[i].cellY == cellY) {
			return static_cast<int>(i);
		}
	}
	return -1;
}

void MockGameClient::collectAt(int cellX, int cellY) {
	const int index = droppedAt(cellX, cellY);
	if (index < 0) {
		return;  // no hay nada para recoger en esa celda
	}
	const world::ResourceKind kind =
		dropped_[static_cast<std::size_t>(index)].kind;
	obtainedKind_ = kind;
	obtainedPending_ = true;
	dropped_.erase(dropped_.begin() + index);
	// Recoger = sumar al inventario. El servidor manda la cantidad; el
	// mock la simula acumulando +1.
	addToInventory(contentIdFor(kind));
}

void MockGameClient::addToInventory(const std::string& contentId) {
	for (ViewInventorySlot& slot : view_.inventory) {
		if (slot.contentId == contentId) {
			++slot.count;
			return;
		}
	}
	// No estaba: ocupar el primer slotIndex libre.
	int index = 0;
	bool used = true;
	while (used) {
		used = false;
		for (const ViewInventorySlot& slot : view_.inventory) {
			if (slot.slotIndex == index) {
				used = true;
				++index;
				break;
			}
		}
	}
	view_.inventory.push_back({index, contentId, 1});
}

std::string MockGameClient::contentIdFor(world::ResourceKind kind) {
	switch (kind) {
	case world::ResourceKind::Silver:
		return "silver";
	case world::ResourceKind::Coal:
		return "coal";
	case world::ResourceKind::Lapis:
		return "lapis";
	case world::ResourceKind::Gold:
		return "gold";
	}
	return "";
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
	rebuildDropped();
}

void MockGameClient::rebuildDropped() const {
	view_.droppedItems.clear();
	for (const DroppedItem& item : dropped_) {
		ViewDroppedItem view;
		// El mock simula minerales: traduce su ResourceKind interno al
		// contentId genérico (mismo vocabulario que el inventario). El
		// servidor mandará directamente el contentId.
		view.contentId = contentIdFor(item.kind);
		view.cellX = item.cellX;
		view.cellY = item.cellY;
		view_.droppedItems.push_back(view);
	}
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
	// El inventario es genérico: cada slot lleva un contentId (string) y
	// una cantidad. Las herramientas arrancan con cantidad 1 (sin número).
	view_.inventory.clear();
	view_.inventory.push_back({0, "pickaxe", 1});
	view_.inventory.push_back({1, "shovel", 1});
	view_.inventory.push_back({2, "watering_can", 1});
	view_.inventory.push_back({3, "sword", 1});
}

}
