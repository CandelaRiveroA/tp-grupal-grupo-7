#pragma once

#include <vector>

#include "game/LocalWorldView.h"
#include "sdl/render/SheetCache.h"
#include "sdl/render/SpriteSheet.h"
#include "world/DigitCatalog.h"
#include "world/InventoryLayout.h"

namespace sdl {
class SdlContext;
}

namespace sdl {

class InventoryHud {
public:
	InventoryHud(SdlContext& context, const world::InventoryLayout& layout,
		     const world::DigitCatalog& digits);

	void draw(const std::vector<game::ViewInventorySlot>& inventory,
		  int activeSlot);

private:
	void drawFrame(const world::FrameLayout& fl, bool drawEdges = true);

	// Dibuja el número 'count' arriba-derecha del slot (solo si count>1).
	void drawCount(const world::PixelBox& slot, int count);

	SdlContext& context_;
	world::InventoryLayout layout_;
	world::DigitCatalog digits_;
	SpriteSheet sheet_;
	SheetCache itemSheets_;
	SpriteSheet digitSheet_;
};

}
