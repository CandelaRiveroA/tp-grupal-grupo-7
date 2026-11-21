#include "sdl/render/NpcPopupPainter.h"

#include <SDL2pp/SDL2pp.hh>

#include "core/Constants.h"
#include "sdl/render/SpriteSheet.h"
#include "sdl/render/TextRenderer.h"

namespace sdl {

NpcPopupPainter::NpcPopupPainter(SdlContext& context, TextRenderer& text)
	: text_(text), sheets_(context) {}

void NpcPopupPainter::draw(const world::NpcDef& npc,
			   const world::PixelBox& panel) {
	const world::NpcPopupStyle& style = npc.popup;
	const world::PixelBox portrait =
		style.placePortrait(npc.portrait.fit(style.portraitBox(panel)));
	SpriteSheet& sheet = sheets_.sheet(
		npc.portrait.sheet, core::assetPath(npc.portrait.sheet),
		npc.portrait.cellWidth, npc.portrait.cellHeight);
	sheet.drawBlock(npc.portrait.at.column, npc.portrait.at.row,
			npc.portrait.span.columns, npc.portrait.span.rows,
			SDL2pp::Rect(portrait.x, portrait.y, portrait.width,
				     portrait.height));

	text_.drawLeftWrapped(npc.text, style.textBox(panel), style.textColor,
			      style.textSize, style.lineGap);
}

}
