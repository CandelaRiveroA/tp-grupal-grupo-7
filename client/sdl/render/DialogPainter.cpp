#include "sdl/render/DialogPainter.h"

#include <SDL2pp/SDL2pp.hh>
#include <cstddef>

#include "core/Constants.h"
#include "sdl/context/SdlContext.h"
#include "sdl/render/Viewport.h"
#include "world/DialogLayout.h"

namespace sdl {

DialogPainter::DialogPainter(SdlContext& context) : sheets_(context) {}

void DialogPainter::draw(const world::DialogCatalog& catalog,
			 const world::Indication indication,
			 const world::PixelRect& entityBox,
			 const int animationTick, const Viewport& viewport) {
	const world::DialogData* data = catalog.find(indication);
	if (data == nullptr) {
		return;
	}

	const world::DialogLayout layout(*data);
	const world::DialogAnimation& animation = data->animation;

	const int frameIndex = animation.frameIndexAt(animationTick);
	const world::SheetCell& cell =
		animation.frames[static_cast<std::size_t>(frameIndex)];
	const world::SpriteVisual& sprite = animation.sprite;

	SpriteSheet& sheet = sheets_.sheet(
		sprite.sheetImage, core::assetPath(sprite.sheetImage),
		sprite.cellWidth, sprite.cellHeight);
	const world::PixelRect dialog =
		viewport.toScreen(layout.dialogRect(entityBox));
	sheet.draw(cell.col, cell.row,
		   SDL2pp::Rect(dialog.x, dialog.y, dialog.width,
				dialog.height));

	// El contenido "informa" al llegar al último frame de la animación
	// (el diálogo ya está abierto): se dibuja centrado adentro.
	const bool settled =
		frameIndex == static_cast<int>(animation.frames.size()) - 1;
	if (!settled) {
		return;
	}

	const world::SpriteVisual& content = data->content;
	SpriteSheet& contentSheet = sheets_.sheet(
		content.sheetImage, core::assetPath(content.sheetImage),
		content.cellWidth, content.cellHeight);
	const world::PixelRect box =
		viewport.toScreen(layout.contentRect(dialog));
	contentSheet.draw(content.col, content.row,
			  SDL2pp::Rect(box.x, box.y, box.width, box.height));
}

}  // namespace sdl
