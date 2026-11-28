#include "sdl/render/HeldItemPainter.h"

#include "core/Constants.h"
#include "sdl/render/SheetCache.h"
#include "sdl/render/SpriteSheet.h"
#include "world/ContentCatalog.h"
#include "world/HeldItemLayout.h"

namespace sdl {

HeldItemPainter::HeldItemPainter(SheetCache& sheets,
				 const world::ContentCatalog& content,
				 const world::CharacterCatalog& characters)
	: sheets_(sheets)
	, content_(content)
	, characters_(characters) {}

void HeldItemPainter::draw(const std::string& contentId,
			   const world::CreatureKind kind,
			   const SDL2pp::Rect& drawBox) const {
	const world::SpriteVisual* visual = content_.find(contentId);
	if (visual == nullptr) {
		return;
	}

	// Tamaño del ítem: el de su sprite (drawn o el nativo de su celda)
	// escalado por la presentación `hold` de la criatura (el cuerpo se
	// dibuja a su escala; el ítem acompaña).
	const world::CreatureData& creature = characters_.dataFor(kind);
	const std::map<std::string, double>::const_iterator extra =
		creature.hold.contentScale.find(contentId);
	const double scale =
		creature.hold.scale *
		(extra == creature.hold.contentScale.end() ? 1.0
							    : extra->second);
	const int itemWidth =
		static_cast<int>(visual->displayWidth() * scale);
	const int itemHeight =
		static_cast<int>(visual->displayHeight() * scale);

	const world::HeldItemLayout layout(
		world::PixelRect{drawBox.x, drawBox.y, drawBox.w, drawBox.h},
		characters_.drawnWidth(kind), characters_.drawnHeight(kind),
		itemWidth, itemHeight, creature.hold.offsetY);
	const world::PixelRect rect = layout.rect();

	SpriteSheet& sheet = sheets_.sheet(
		visual->sheetImage, core::assetPath(visual->sheetImage),
		visual->cellWidth, visual->cellHeight);
	// drawBlock: un mueble abarca varias celdas de la hoja (span); un
	// ítem común es el bloque de 1x1 (mismo resultado que antes).
	sheet.drawBlock(visual->col, visual->row, visual->cols, visual->rows,
			SDL2pp::Rect(rect.x, rect.y, rect.width, rect.height));
}

}
