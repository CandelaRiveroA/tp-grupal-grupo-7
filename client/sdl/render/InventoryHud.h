#pragma once

#include <vector>

#include "game/LocalWorldView.h"
#include "sdl/render/DigitPainter.h"
#include "sdl/render/SheetCache.h"
#include "sdl/render/SpriteSheet.h"
#include "world/InventoryLayout.h"

namespace sdl {
class SdlContext;
}

namespace sdl {

class InventoryHud {
public:
	InventoryHud(SdlContext& context, const world::InventoryLayout& layout,
		     DigitPainter& digits);

	void draw(const std::vector<game::ViewInventorySlot>& inventory,
		  int activeSlot, bool trashHovered);

	// La figura del inventario "vacía" en la caja pedida: los cuadros
	// de los slots (el fondo del panel, en grilla columns×rows) y el
	// marco. Sin slots ni herramientas.
	void drawPanel(const world::PixelBox& box, int columns, int rows);

	// La figura completa del inventario en la caja pedida: el panel
	// (drawPanel) y, encima, los slots con lo mismo que la barra (marco
	// de cada slot, íconos y cantidades). `activeSlot` < 0 = sin marcar.
	void drawGrid(const world::PixelBox& box, int columns, int rows,
		      const std::vector<game::ViewInventorySlot>& slots,
		      int activeSlot);

private:
	void drawSlotBackground(const world::PixelBox& box);

	void drawTrash(bool hovered);

	// Marco de cada slot, marco del activo, íconos y cantidades sobre las
	// cajas dadas (compartido por la barra y por cualquier grilla).
	void drawSlotContents(
		const std::vector<world::PixelBox>& boxes,
		const std::vector<game::ViewInventorySlot>& inventory,
		int activeSlot);

	void drawFrame(const world::FrameLayout& fl, bool drawEdges = true);

	// Dibuja el número 'count' arriba-derecha del slot (solo si count>1).
	void drawCount(const world::PixelBox& slot, int count);

	SdlContext& context_;
	world::InventoryLayout layout_;
	DigitPainter& digits_;
	SpriteSheet sheet_;
	SheetCache itemSheets_;
};

}
