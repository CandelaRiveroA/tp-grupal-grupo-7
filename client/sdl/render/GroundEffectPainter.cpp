#include "sdl/render/GroundEffectPainter.h"

#include <SDL2pp/SDL2pp.hh>
#include <map>
#include <utility>

#include "core/Constants.h"
#include "sdl/render/SpriteSheet.h"
#include "sdl/render/Viewport.h"
#include "world/CellPatch.h"
#include "world/WorldConstants.h"

namespace sdl {

GroundEffectPainter::GroundEffectPainter(SheetCache& sheets,
					 const world::GroundCatalog& ground)
	: sheets_(sheets)
	, ground_(ground) {}

void GroundEffectPainter::draw(
	const std::vector<game::ViewGroundEffect>& effects,
	const world::ScenePixelMapper& mapper, const Viewport& viewport) const {
	// Un parche por consecuencia: los vecinos que arman una zona son
	// los de la MISMA consecuencia (la tierra tratada se arma con
	// tierra tratada; el regado futuro se armará con regado).
	std::map<world::GroundConsequence, std::vector<world::GridCell>>
		cellsByConsequence;
	for (const game::ViewGroundEffect& effect : effects) {
		cellsByConsequence[effect.consequence].push_back(
			world::GridCell{effect.cellX, effect.cellY});
	}
	std::map<world::GroundConsequence, world::CellPatch> patches;
	for (const auto& [consequence, cells] : cellsByConsequence) {
		patches.emplace(consequence, world::CellPatch(cells));
	}

	for (const game::ViewGroundEffect& effect : effects) {
		const world::GridCell cell{effect.cellX, effect.cellY};
		const world::SpriteVisual* visual = ground_.pieceFor(
			effect.consequence,
			patches.at(effect.consequence).maskAt(cell), cell);
		if (visual == nullptr) {
			continue;
		}

		const world::PixelRect rect = viewport.toScreen(mapper.toPixels(
			cell, world::kUnitCellSpan));
		if (!viewport.intersects(rect)) {
			continue;
		}

		SpriteSheet& sheet = sheets_.sheet(
			visual->sheetImage, core::assetPath(visual->sheetImage),
			visual->cellWidth, visual->cellHeight);
		sheet.draw(visual->col, visual->row,
			   SDL2pp::Rect(rect.x, rect.y, rect.width, rect.height));
	}
}

}
