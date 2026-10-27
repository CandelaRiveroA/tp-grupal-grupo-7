#include "sdl/render/InventoryHud.h"

#include <vector>

#include "core/Constants.h"
#include "sdl/context/SdlContext.h"

namespace sdl {

InventoryHud::InventoryHud(SdlContext& context,
			   const world::InventoryLayout& layout)
	: context_(context)
	, layout_(layout)
	, sheet_(context, core::assetPath(layout.sheetImage),
		 layout.cellWidth, layout.cellHeight)
	, itemSheets_(context) {}

void InventoryHud::drawFrame(const world::FrameLayout& fl, bool drawEdges) {
	auto drawPiece = [&](const world::FrameLayout::Piece& p) {
		sheet_.drawBlock(
			p.cell.col, p.cell.row, p.srcCols, p.srcRows,
			SDL2pp::Rect(p.dest.x, p.dest.y, p.dest.width,
				     p.dest.height));
	};
	for (const world::FrameLayout::Piece& corner : fl.corners) {
		drawPiece(corner);
	}
	if (drawEdges) {
		for (const world::FrameLayout::Piece& edge : fl.edges) {
			drawPiece(edge);
		}
	}
}

void InventoryHud::draw(
	const std::vector<game::ViewInventorySlot>& inventory, int activeSlot) {
	const std::vector<world::PixelBox> boxes =
		layout_.slotBoxes(context_.width(), context_.height());

	for (const world::PixelBox& box : boxes) {
		sheet_.draw(layout_.slotCol, layout_.slotRow,
			    SDL2pp::Rect(box.x, box.y, box.width, box.height));
	}

	if (layout_.frame.enabled) {
		drawFrame(layout_.frameLayout(context_.width(),
					      context_.height()));
	}

	if (layout_.slotFrame.enabled) {
		const bool drawEdges = !layout_.slotFrame.cornersOnly;
		for (const world::PixelBox& box : boxes) {
			drawFrame(layout_.slotFrameLayout(box), drawEdges);
		}
	}

	if (layout_.activeSlotFrame.enabled) {
		const int clampedSlot =
			(activeSlot >= 0 && activeSlot < static_cast<int>(boxes.size()))
				? activeSlot
				: 0;
		const bool drawEdges = !layout_.activeSlotFrame.cornersOnly;
		drawFrame(layout_.activeSlotFrameLayout(
				  boxes[static_cast<std::size_t>(clampedSlot)]),
			  drawEdges);
	}

	const int slots = static_cast<int>(boxes.size());
	for (const game::ViewInventorySlot& entry : inventory) {
		if (entry.slotIndex < 0 || entry.slotIndex >= slots) {
			continue;
		}
		if (!layout_.items.has(entry.item)) {
			continue;
		}
		const world::ItemVisual& visual =
			layout_.items.visualFor(entry.item);
		SpriteSheet& itemSheet = itemSheets_.sheet(
			visual.sheetImage, core::assetPath(visual.sheetImage),
			visual.cellWidth, visual.cellHeight);
		const world::PixelBox icon =
			layout_.iconBox(boxes[static_cast<std::size_t>(
				entry.slotIndex)]);
		itemSheet.draw(visual.col, visual.row,
			       SDL2pp::Rect(icon.x, icon.y, icon.width,
					    icon.height));
	}
}

}
