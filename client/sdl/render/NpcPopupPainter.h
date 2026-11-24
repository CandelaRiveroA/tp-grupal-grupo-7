#pragma once

#include "sdl/render/SheetCache.h"
#include "world/NpcCatalog.h"

namespace sdl {
class SdlContext;
class TextRenderer;
}

namespace sdl {

// Pinta el retrato y el texto del popup de un NPC. El panel lo dibuja
// InventoryHud; esto solo llena las cajas que calcula NpcPopupStyle.
class NpcPopupPainter {
public:
	NpcPopupPainter(SdlContext& context, TextRenderer& text);

	void draw(const world::NpcDef& npc, const world::PixelBox& panel);

private:
	TextRenderer& text_;
	SheetCache sheets_;
};

}
