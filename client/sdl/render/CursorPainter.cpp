#include "sdl/render/CursorPainter.h"

#include <SDL2pp/SDL2pp.hh>

#include "core/Constants.h"
#include "sdl/context/SdlContext.h"

namespace sdl {

CursorPainter::CursorPainter(SdlContext& context) : sheets_(context) {}

void CursorPainter::draw(const world::CursorCatalog& catalog,
			 const world::CursorKind kind,
			 const game::PointerPosition& position) {
	const world::SpriteVisual* visual = catalog.find(kind);
	if (visual == nullptr) {
		return;
	}

	// Tamaño en pantalla: drawn si lo declara, si no el nativo de la
	// celda de la hoja.
	const int width = visual->drawnWidth > 0 ? visual->drawnWidth
						 : visual->cellWidth;
	const int height = visual->drawnHeight > 0 ? visual->drawnHeight
						  : visual->cellHeight;

	// El hotspot es relativo al sprite nativo: escala con el tamaño
	// dibujado para que el punto de punta no se corra al escalar.
	const int hotspotX = visual->cellWidth > 0
		? catalog.hotspot.x * width / visual->cellWidth
		: catalog.hotspot.x;
	const int hotspotY = visual->cellHeight > 0
		? catalog.hotspot.y * height / visual->cellHeight
		: catalog.hotspot.y;

	SpriteSheet& sheet = sheets_.sheet(
		visual->sheetImage, core::assetPath(visual->sheetImage),
		visual->cellWidth, visual->cellHeight);
	sheet.draw(visual->col, visual->row,
		   SDL2pp::Rect(position.x - hotspotX, position.y - hotspotY,
				width, height));
}

}  // namespace sdl
