#include "sdl/render/BuildPlacePainter.h"

#include <SDL2pp/SDL2pp.hh>

#include "core/Constants.h"
#include "sdl/context/SdlContext.h"
#include "sdl/render/SpriteSheet.h"
#include "sdl/render/Viewport.h"
#include "world/BuildPlaceLayout.h"
#include "world/WorldConstants.h"

namespace sdl {

BuildPlacePainter::BuildPlacePainter(SdlContext& context) : sheets_(context) {}

void BuildPlacePainter::draw(const world::BuildPlaceCatalog& catalog,
			     const world::BuildPlaceState state,
			     const world::GridCell& cell,
			     const world::ScenePixelMapper& mapper,
			     const Viewport& viewport) {
	const world::BuildPlaceVisual* visual = catalog.find(state);
	if (visual == nullptr) {
		return;
	}

	const world::BuildPlaceLayout layout;
	const world::PixelRect rect = viewport.toScreen(layout.rectFor(
		mapper.toPixels(cell, world::kUnitCellSpan), *visual));
	if (!viewport.intersects(rect)) {
		return;
	}

	SpriteSheet& sheet = sheets_.sheet(
		visual->sprite.sheetImage,
		core::assetPath(visual->sprite.sheetImage),
		visual->sprite.cellWidth, visual->sprite.cellHeight);
	sheet.drawBlock(visual->sprite.col, visual->sprite.row, visual->cols,
			visual->rows,
			SDL2pp::Rect(rect.x, rect.y, rect.width, rect.height));
}

}
