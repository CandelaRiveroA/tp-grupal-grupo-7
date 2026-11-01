#include "sdl/render/ToolOverlayPainter.h"

#include "core/Constants.h"
#include "game/LocalWorldView.h"
#include "sdl/render/RenderConstants.h"
#include "sdl/render/SheetCache.h"
#include "sdl/render/SpriteSheet.h"
#include "world/CharacterLayout.h"
#include "world/ToolOverlayLayout.h"

namespace sdl {

ToolOverlayPainter::ToolOverlayPainter(
	SheetCache& sheets, const world::ToolOverlayCatalog& toolOverlay,
	const world::CharacterCatalog& characters)
	: sheets_(sheets)
	, toolOverlay_(toolOverlay)
	, characters_(characters) {}

void ToolOverlayPainter::draw(const game::ViewEntity& entity,
			      const SDL2pp::Rect& drawBox, const int phase,
			      const world::ActionKind action,
			      const world::Pose& pose) const {
	if (!entity.activeTool.has_value()) {
		return;
	}

	const world::ItemKind tool = *entity.activeTool;
	const world::Direction dir = entity.facing;
	const world::ToolOverlayAction* found = toolOverlay_.find(tool, dir);
	if (found == nullptr) {
		return;
	}

	const world::ToolOverlayAction& overlayAction = *found;

	// Cada overlay declara con qué acción se sincroniza (dato, no
	// código): el mismo mecanismo dibuja la herramienta en use_tool y
	// el huevo en las manos al recoger.
	if (overlayAction.action != action) {
		return;
	}

	const world::CharacterLayout body(
		world::PixelRect{drawBox.x, drawBox.y, drawBox.w, drawBox.h},
		characters_.drawnWidth(entity.kind),
		characters_.drawnHeight(entity.kind));
	// La capa base del pose es la primera (el cuerpo), el mismo
	// convenio que CharacterSprite.
	const world::ToolOverlayLayout layout(body, toolOverlay_,
					      pose.front());

	const world::ToolOverlayFrame* frame =
		layout.frameAt(overlayAction, phase);
	if (frame == nullptr) {
		return;
	}

	SpriteSheet& sheet = sheets_.sheet(
		overlayAction.sheetImage,
		core::assetPath(overlayAction.sheetImage),
		overlayAction.cellWidth, overlayAction.cellHeight);

	for (const world::LayerFrame& layer : frame->layers) {
		const world::PixelRect rect = layout.layerRect(*frame, layer);
		const SDL2pp::Point pivot(
			static_cast<int>(frame->pivotX * rect.width),
			static_cast<int>(frame->pivotY * rect.height));
		const Flip flip =
			frame->flip ? Flip::Horizontal : Flip::None;
		sheet.drawBlock(layer.frame.col, layer.frame.row, layer.cols,
				layer.rows,
				SDL2pp::Rect(rect.x, rect.y, rect.width,
					     rect.height),
				flip, render::kFullSourceFraction,
				render::kFullSourceFraction, frame->angle,
				pivot);
	}
}

}  // namespace sdl
