#include "sdl/render/InventoryHud.h"

#include <algorithm>
#include <vector>

#include "core/Constants.h"
#include "sdl/context/SdlContext.h"

namespace sdl {

InventoryHud::InventoryHud(SdlContext& context,
			   const world::InventoryLayout& layout,
			   DigitPainter& digits)
	: context_(context)
	, layout_(layout)
	, digits_(digits)
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
	const std::vector<game::ViewInventorySlot>& inventory, int activeSlot,
	const bool trashHovered) {
	const std::vector<world::PixelBox> boxes =
		layout_.slotBoxes(context_.width(), context_.height());

	for (const world::PixelBox& box : boxes) {
		drawSlotBackground(box);
	}

	if (layout_.frame.enabled) {
		drawFrame(layout_.frameLayout(context_.width(),
					      context_.height()));
	}

	drawSlotContents(boxes, inventory, activeSlot);
	drawTrash(trashHovered);
}

void InventoryHud::drawSlotBackground(const world::PixelBox& box) {
	sheet_.draw(layout_.slotCol, layout_.slotRow,
		    SDL2pp::Rect(box.x, box.y, box.width, box.height));
}

void InventoryHud::drawTrash(const bool hovered) {
	const world::TrashButton& trash = layout_.trash;
	if (!trash.enabled) {
		return;
	}
	const int width = context_.width();
	const int height = context_.height();
	const world::PixelBox inner = layout_.trashInnerBox(width, height);
	drawSlotBackground(inner);
	const world::PixelBox icon = layout_.trashIconBox(width, height);
	SpriteSheet& sheet = itemSheets_.sheet(trash.sheetImage,
					       core::assetPath(trash.sheetImage),
					       trash.cellWidth, trash.cellHeight);
	sheet.draw(0, 0, SDL2pp::Rect(icon.x, icon.y, icon.width, icon.height));
	if (hovered) {
		const world::Color& tint = trash.hoverTint;
		context_.fillRectMultiply(
			SDL2pp::Rect(inner.x, inner.y, inner.width, inner.height),
			SDL2pp::Color(tint.r, tint.g, tint.b, 255));
	}
	if (layout_.frame.enabled) {
		drawFrame(layout_.frameLayout(layout_.trashBox(width, height)));
	}
}

void InventoryHud::drawSlotContents(
	const std::vector<world::PixelBox>& boxes,
	const std::vector<game::ViewInventorySlot>& inventory, int activeSlot) {
	if (layout_.slotFrame.enabled) {
		const bool drawEdges = !layout_.slotFrame.cornersOnly;
		for (const world::PixelBox& box : boxes) {
			drawFrame(layout_.slotFrameLayout(box), drawEdges);
		}
	}

	const int slots = static_cast<int>(boxes.size());

	if (layout_.activeSlotFrame.enabled && activeSlot >= 0 &&
	    activeSlot < slots) {
		// El marco marca un slot CON contenido: la selección nunca
		// se dibuja sobre un cuadro vacío. Qué hay es estado (la
		// vista); esto solo dibuja lo que la vista indica.
		const bool occupied = std::any_of(
			inventory.begin(), inventory.end(),
			[&](const game::ViewInventorySlot& entry) {
				return entry.slotIndex == activeSlot;
			});
		if (occupied) {
			const bool drawEdges = !layout_.activeSlotFrame.cornersOnly;
			drawFrame(layout_.activeSlotFrameLayout(
					  boxes[static_cast<std::size_t>(activeSlot)]),
				  drawEdges);
		}
	}
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
		// Tamaño en la celda: el `slot_drawn` del ítem si lo declara,
		// si no el `drawn` del sprite (centrado), si no el sprite
		// ajustado a la caja sin deformarse (los de celda 16x16, por
		// ejemplo, la llenan).
		world::PixelBox target = visual->fitIn(icon);
		int drawW = 0;
		int drawH = 0;
		const auto slotIt = layout_.slotDrawn.find(entry.contentId);
		if (slotIt != layout_.slotDrawn.end()) {
			drawW = slotIt->second.width;
			drawH = slotIt->second.height;
		} else if (visual->drawnWidth > 0 && visual->drawnHeight > 0) {
			drawW = visual->displayWidth();
			drawH = visual->displayHeight();
		}
		if (drawW > 0 && drawH > 0) {
			target = world::PixelBox{
				icon.x + (icon.width - drawW) / 2,
				icon.y + (icon.height - drawH) / 2, drawW,
				drawH};
		}
		itemSheet.drawBlock(visual->col, visual->row, visual->cols,
				    visual->rows,
				    SDL2pp::Rect(target.x, target.y,
						 target.width, target.height));
		// Cantidad: solo se dibuja si hay más de 1.
		if (entry.count > 1) {
			drawCount(slotBox, entry.count);
		}
	}
}

void InventoryHud::drawGrid(
	const world::PixelBox& box, const int columns, const int rows,
	const std::vector<game::ViewInventorySlot>& slots, int activeSlot) {
	// La figura del inventario en la caja pedida: fondo y marco del
	// panel y, encima, lo mismo que dibuja la barra en cada slot.
	drawPanel(box, columns, rows);
	drawSlotContents(layout_.slotBoxes(box, columns, rows), slots,
			 activeSlot);
}

void InventoryHud::drawPanel(const world::PixelBox& box, const int columns,
			     const int rows) {
	// La figura del inventario "vacía": los cuadros de los slots (el
	// fondo del panel, en grilla columns×rows) y, encima, el marco.
	// Sin el borde interior de cada slot (la profundidad) y sin ítems.
	for (const world::PixelBox& slot : layout_.slotBoxes(box, columns,
								rows)) {
		drawSlotBackground(slot);
	}
	if (layout_.frame.enabled) {
		drawFrame(layout_.frameLayout(box));
	}
}

void InventoryHud::drawCount(const world::PixelBox& slot, int count) {
	const std::vector<int> digits = world::digitsOf(count);
	const std::vector<world::PixelBox> boxes =
		layout_.countBoxes(slot, static_cast<int>(digits.size()));
	digits_.draw(digits, boxes);
}

}
