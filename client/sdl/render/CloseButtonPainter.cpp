#include "sdl/render/CloseButtonPainter.h"

#include <SDL2pp/SDL2pp.hh>

#include "core/Constants.h"
#include "sdl/render/SpriteSheet.h"

namespace sdl {

CloseButtonPainter::CloseButtonPainter(SdlContext& context)
	: sheets_(context) {}

void CloseButtonPainter::draw(const world::CloseButton& button,
			      const world::PixelBox& panel) {
	const world::PixelBox box = button.box(panel);
	SpriteSheet& sheet =
		sheets_.sheet(button.sheet, core::assetPath(button.sheet),
			      button.cellWidth, button.cellHeight);
	sheet.draw(button.cell.column, button.cell.row,
		   SDL2pp::Rect(box.x, box.y, box.width, box.height));
}

}
