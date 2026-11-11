#include "sdl/render/CropPainter.h"

#include <SDL2pp/SDL2pp.hh>

#include "core/Constants.h"
#include "sdl/render/SpriteSheet.h"
#include "sdl/render/Viewport.h"
#include "world/WorldConstants.h"

namespace sdl {

CropPainter::CropPainter(SheetCache& sheets, const world::CropCatalog& crops)
	: sheets_(sheets)
	, crops_(crops) {}

void CropPainter::draw(const std::vector<game::ViewCrop>& planted,
		       const world::ScenePixelMapper& mapper,
		       const Viewport& viewport) const {
	for (const game::ViewCrop& crop : planted) {
		// El cultivo y su etapa los dice la vista; el sprite sale
		// de su catálogo (etapa 0 = semilla, 1..5 = crecimientos).
		const world::CropDefinition* definition =
			crops_.find(crop.cropId);
		if (definition == nullptr) {
			continue;
		}
		const world::SpriteVisual* visual =
			definition->spriteForStage(crop.stage, crop.quality);
		if (visual == nullptr) {
			continue;
		}

		const world::GridCell cell{crop.cellX, crop.cellY};
		const world::PixelRect rect = viewport.toScreen(
			mapper.toPixels(cell, world::kUnitCellSpan));
		if (!viewport.intersects(rect)) {
			continue;
		}

		// El contenido de la planta (sin su aire transparente) se
		// dibuja a su escala de diseño (con la del catálogo
		// encima) y centrado en la celda de tierra: la semilla
		// cae en el centro del cosito con el aire repartido
		// parejo, sin importar de dónde venga dibujada dentro de
		// su celda en la hoja. La cosecha va una dimensión menos
		// (su fruto ya llena su celda).
		const double scale =
			crop.stage == definition->finalStage()
				? crops_.harvestScale()
				: crops_.plantScale();
		SpriteSheet& sheet = sheets_.sheet(
			visual->sheetImage, core::assetPath(visual->sheetImage),
			visual->cellWidth, visual->cellHeight);
		sheet.drawContentCentered(visual->col, visual->row,
					  SDL2pp::Rect(rect.x, rect.y,
						       rect.width,
						       rect.height),
					  scale);
	}
}

}  // namespace sdl
