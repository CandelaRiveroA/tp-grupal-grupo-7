#include "game/ContainerSession.h"

#include "game/GameConstants.h"

namespace game {

ContainerSession::ContainerSession(const world::InventoryLayout& layout)
	: layout_(layout) {}

void ContainerSession::open(const std::string& containerId) {
	containerId_ = containerId;
	selected_ = -1;
}

void ContainerSession::close() {
	containerId_.clear();
	selected_ = -1;
}

bool ContainerSession::hover(const world::PixelPoint& point,
			     const world::PixelBox& panel,
			     const world::GridSpan& grid,
			     const LocalWorldView& view) {
	const int slot = layout_.slotAt(panel, grid.columns, grid.rows, point);
	const int previous = selected_;
	selected_ = slot >= 0 &&
				    stacker_.find(view.containerSlots, slot) !=
					    nullptr
			    ? slot
			    : -1;
	return selected_ >= 0 && selected_ != previous;
}

int ContainerSession::selected() const {
	return selected_;
}

ContainerTransfer ContainerSession::transfer(
	const TransferDirection direction, const int slotIndex) const {
	return ContainerTransfer{containerId_, direction, slotIndex,
				 kTransferAmount};
}

std::optional<ContainerTransfer> ContainerSession::click(
	const world::PixelPoint& point, const world::PixelBox& panel,
	const world::GridSpan& grid, const LocalWorldView& view,
	const int windowWidth, const int windowHeight) {
	const int containerSlot =
		layout_.slotAt(panel, grid.columns, grid.rows, point);
	if (containerSlot >= 0) {
		if (stacker_.find(view.containerSlots, containerSlot) ==
		    nullptr) {
			return std::nullopt;
		}
		return transfer(TransferDirection::ToInventory, containerSlot);
	}
	const int inventorySlot =
		layout_.slotAt(windowWidth, windowHeight, point);
	if (inventorySlot >= 0 &&
	    stacker_.find(view.inventory, inventorySlot) != nullptr) {
		return transfer(TransferDirection::ToContainer, inventorySlot);
	}
	return std::nullopt;
}

bool ContainerSession::overItem(const world::PixelPoint& point,
				const world::PixelBox& panel,
				const world::GridSpan& grid,
				const LocalWorldView& view,
				const int windowWidth,
				const int windowHeight) const {
	const int containerSlot =
		layout_.slotAt(panel, grid.columns, grid.rows, point);
	if (containerSlot >= 0) {
		return stacker_.find(view.containerSlots, containerSlot) !=
		       nullptr;
	}
	const int inventorySlot =
		layout_.slotAt(windowWidth, windowHeight, point);
	return inventorySlot >= 0 &&
	       stacker_.find(view.inventory, inventorySlot) != nullptr;
}

}
