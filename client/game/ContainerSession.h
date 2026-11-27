#pragma once

#include <optional>
#include <string>

#include "game/ContainerTransfer.h"
#include "game/LocalWorldView.h"
#include "game/SlotStacker.h"
#include "world/Grid.h"
#include "world/InventoryLayout.h"

namespace game {

class ContainerSession {
public:
	explicit ContainerSession(const world::InventoryLayout& layout);

	void open(const std::string& containerId);
	void close();

	bool hover(const world::PixelPoint& point, const world::PixelBox& panel,
		   const world::GridSpan& grid, const LocalWorldView& view);

	int selected() const;

	std::optional<ContainerTransfer> click(
		const world::PixelPoint& point, const world::PixelBox& panel,
		const world::GridSpan& grid, const LocalWorldView& view,
		int windowWidth, int windowHeight);

	bool overItem(const world::PixelPoint& point,
		      const world::PixelBox& panel, const world::GridSpan& grid,
		      const LocalWorldView& view, int windowWidth,
		      int windowHeight) const;

private:
	ContainerTransfer transfer(TransferDirection direction,
				   int slotIndex) const;

	const world::InventoryLayout& layout_;
	SlotStacker stacker_;
	std::string containerId_;
	int selected_ = -1;
};

}
