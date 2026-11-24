#pragma once

#include <string>
#include <vector>

#include "sdl/render/SheetCache.h"
#include "world/CommunityBoard.h"
#include "world/CommunityPopupStyle.h"

namespace sdl {
class SdlContext;
class TextRenderer;
}

namespace sdl {

// Pinta el contenido del popup del Centro Comunitario (título, sets con su
// estado, checkboxes y cantidades) de las secciones que le pasen, con el
// estilo compartido. El panel lo dibuja InventoryHud; las cajas las
// calcula world::CommunityPopupStyle y qué mostrar ya viene resuelto
// (game::CommunityBoardResolver): esto solo pinta.
class CommunityPopupPainter {
public:
	CommunityPopupPainter(SdlContext& context, TextRenderer& text);

	void draw(const world::CommunityPopupStyle& style,
		  const std::string& title,
		  const std::vector<world::CommunitySection>& sections,
		  const world::PixelBox& panel);

private:
	void drawHeader(const world::CommunityPopupStyle& style,
			const world::CommunitySection& section,
			const world::PixelBox& header);
	void drawRow(const world::CommunityPopupStyle& style,
		     const world::CommunityRow& row,
		     const world::PixelBox& box);
	void drawVisual(const world::SpriteVisual& visual,
			const world::PixelBox& box);

	TextRenderer& text_;
	SheetCache sheets_;
};

}
