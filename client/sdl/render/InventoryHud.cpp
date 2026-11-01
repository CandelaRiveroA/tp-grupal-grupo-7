#include "sdl/render/InventoryHud.h"

#include <vector>

#include "core/Constants.h"
#include "sdl/context/SdlContext.h"

namespace sdl {

InventoryHud::InventoryHud(SdlContext& context,
			   const world::InventoryLayout& layout,
			   const world::DigitCatalog& digits)
	: context_(context)
	, layout_(layout)
	, digits_(digits)
	, sheet_(context, core::assetPath(layout.sheetImage),
		 layout.cellWidth, layout.cellHeight)
	, itemSheets_(context)
	, digitSheet_(context, core::assetPath(digits.sheetImage),
		      digits.cellWidth, digits.cellHeight) {}

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
		const world::SpriteVisual* visual =
			layout_.content.find(entry.contentId);
		if (visual == nullptr) {
			continue;
		}
		SpriteSheet& itemSheet = itemSheets_.sheet(
			visual->sheetImage,
			core::assetPath(visual->sheetImage),
			visual->cellWidth, visual->cellHeight);
		const world::PixelBox& slotBox =
			boxes[static_cast<std::size_t>(entry.slotIndex)];
		const world::PixelBox icon = layout_.iconBox(slotBox);
		itemSheet.draw(visual->col, visual->row,
			       SDL2pp::Rect(icon.x, icon.y, icon.width,
					    icon.height));
		// Cantidad: solo se dibuja si hay más de 1.
		if (entry.count > 1) {
			drawCount(slotBox, entry.count);
		}
	}
}

void InventoryHud::drawCount(const world::PixelBox& slot, int count) {
	const std::vector<int> digits = world::digitsOf(count);
	const std::vector<world::PixelBox> boxes =
		layout_.countBoxes(slot, static_cast<int>(digits.size()));
	// Los dígitos de la hoja son claros; se tiñen de negro al dibujarlos.
	digitSheet_.setColorMod(0, 0, 0);
	for (std::size_t i = 0; i < digits.size() && i < boxes.size(); ++i) {
		const world::SheetCell& cell = digits_.cellFor(digits[i]);
		const world::PixelBox& box = boxes[i];
		digitSheet_.draw(cell.col, cell.row,
				 SDL2pp::Rect(box.x, box.y, box.width,
					      box.height));
	}
	digitSheet_.clearColorMod();
}

}
