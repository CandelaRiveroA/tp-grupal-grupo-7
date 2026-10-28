#pragma once

#include <vector>

#include "game/LocalWorldView.h"
#include "sdl/render/SheetCache.h"
#include "sdl/render/SpriteSheet.h"
#include "world/InventoryLayout.h"

namespace sdl {
class SdlContext;
}

namespace sdl {

class InventoryHud {
public:
	InventoryHud(SdlContext& context, const world::InventoryLayout& layout);

	void draw(const std::vector<game::ViewInventorySlot>& inventory);

private:
	void drawFrame(const world::FrameLayout& fl, bool drawEdges = true);

	SdlContext& context_;
	world::InventoryLayout layout_;
	SpriteSheet sheet_;
	SheetCache itemSheets_;
};

}
