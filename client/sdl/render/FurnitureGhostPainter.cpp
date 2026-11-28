#include "sdl/render/FurnitureGhostPainter.h"

#include <optional>

#include <SDL2pp/SDL2pp.hh>

#include "core/Constants.h"
#include "sdl/render/SheetCache.h"
#include "sdl/render/SpriteSheet.h"
#include "sdl/render/Viewport.h"
#include "world/FurnitureLayout.h"

namespace sdl {

FurnitureGhostPainter::FurnitureGhostPainter(SheetCache& sheets,
					     const world::FurnitureGeometry& furniture)
	: sheets_(sheets), furniture_(furniture), props_(sheets) {}

void FurnitureGhostPainter::draw(const std::string& contentId,
				 const world::GridCell& base,
				 const world::FurnitureFacing facing,
				 const world::GridCell& toward,
				 const double push, const int alpha,
				 const world::ScenePixelMapper& mapper,
				 const Viewport& viewport) {
	const std::optional<world::FurnitureShape> shape =
		furniture_.shapeAtBase(contentId, base, mapper, facing);
	if (!shape.has_value()) {
		return;
	}
	const world::PixelRect pushed =
		world::FurnitureLayout(mapper).pushed(shape->box, toward, push);
	if (const world::NpcDef* prop = furniture_.prop(contentId); prop != nullptr) {
		PropDraw how;
		how.where = shape->topLeft;
		how.alpha = alpha;
		how.shift = world::PixelPoint{pushed.x - shape->box.x, pushed.y - shape->box.y};
		props_.draw(*prop, mapper.cellPixelSize(), how, viewport);
		return;
	}
	const world::SpriteVisual* visual = shape->visual;
	const world::PixelRect rect = viewport.toScreen(pushed);
	if (visual == nullptr || !viewport.intersects(rect)) {
		return;
	}
	SpriteSheet& sheet = sheets_.sheet(
		visual->sheetImage, core::assetPath(visual->sheetImage),
		visual->cellWidth, visual->cellHeight);
	sheet.setAlphaMod(static_cast<std::uint8_t>(alpha));
	sheet.drawBlock(visual->col, visual->row, visual->cols, visual->rows,
			SDL2pp::Rect(rect.x, rect.y, rect.width, rect.height),
			shape->mirrored ? Flip::Horizontal : Flip::None);
	sheet.clearAlphaMod();
}

}
