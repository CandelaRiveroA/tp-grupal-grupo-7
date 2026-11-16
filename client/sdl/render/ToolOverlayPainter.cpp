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
	const world::CharacterCatalog& characters,
	const world::ContentCatalog& content)
	: sheets_(sheets)
	, toolOverlay_(toolOverlay)
	, characters_(characters)
	, content_(content) {}

void ToolOverlayPainter::draw(const game::ViewEntity& entity,
			      const SDL2pp::Rect& drawBox, const int phase,
			      const world::ActionKind action,
			      const world::Pose& pose,
			      const world::OverlayPass pass) const {
	// Precondición (como CharacterSprite::draw): una pose vacía es un
	// frame "invisible" (la titulación del daño) y no tiene capa base.
	if (pose.empty()) {
		return;
	}
	if (!entity.activeTool.has_value() &&
	    !entity.activeContent.has_value()) {
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
	if (entity.activeTool.has_value()) {
		drawTool(entity, phase, action, layout, pass);
	}
	if (entity.activeContent.has_value()) {
		drawHeldContent(entity, phase, action, layout, pass);
	}
}

void ToolOverlayPainter::drawTool(const game::ViewEntity& entity,
				  const int phase,
				  const world::ActionKind action,
				  const world::ToolOverlayLayout& layout,
				  const world::OverlayPass pass) const {
	const world::ItemKind tool = *entity.activeTool;
	const world::Direction dir = entity.facing;
	const world::ToolOverlayAction* found = toolOverlay_.find(tool, dir);
	if (found == nullptr) {
		return;
	}

	const world::ToolOverlayAction& overlayAction = *found;

	// Cada overlay declara con qué acción se sincroniza (dato, no
	// código): el mismo mecanismo dibuja la herramienta en use_tool.
	if (overlayAction.action != action) {
		return;
	}

	// La herramienta y su efecto (el chorro de la regadera) se dibujan
	// con el MISMO mecanismo: ambos son AnimatedOverlay. El efecto es
	// opcional y puede sincronizarse con otra acción (dato); si está y
	// corresponde a la acción en curso, se pinta encima.
	drawOverlayFrames(overlayAction, phase, layout, pass);
	if (overlayAction.effect.has_value() &&
	    overlayAction.effect->action == action) {
		drawOverlayFrames(*overlayAction.effect, phase, layout, pass);
	}
}

void ToolOverlayPainter::drawOverlayFrames(
	const world::AnimatedOverlay& overlay, const int phase,
	const world::ToolOverlayLayout& layout,
	const world::OverlayPass pass) const {
	const world::ToolOverlayFrame* frame = layout.frameAt(overlay, phase, pass);
	if (frame == nullptr) {
		return;
	}

	SpriteSheet& sheet = sheets_.sheet(
		overlay.sheetImage, core::assetPath(overlay.sheetImage),
		overlay.cellWidth, overlay.cellHeight);

	for (const world::OverlayLayer& layer : frame->layers) {
		const world::PixelRect rect = layout.layerRect(*frame, layer);
		const SDL2pp::Point pivot(
			static_cast<int>(frame->pivotX * rect.width),
			static_cast<int>(frame->pivotY * rect.height));
		const Flip flip =
			frame->flip ? Flip::Horizontal : Flip::None;
		sheet.drawBlock(layer.layer.frame.col, layer.layer.frame.row,
				layer.layer.cols, layer.layer.rows,
				SDL2pp::Rect(rect.x, rect.y, rect.width,
					     rect.height),
				flip, render::kFullSourceFraction,
				render::kFullSourceFraction, frame->angle,
				pivot);
	}
}

void ToolOverlayPainter::drawHeldContent(
	const game::ViewEntity& entity, const int phase,
	const world::ActionKind action,
	const world::ToolOverlayLayout& layout,
	const world::OverlayPass pass) const {
	const world::HeldContentOverlay* overlay =
		toolOverlay_.findHeldContent(entity.facing);
	if (overlay == nullptr || overlay->action != action) {
		return;
	}

	// El sprite es el del contenido sostenido (el MISMO del
	// inventario), resuelto por contentId: una cosecha nueva no
	// agrega código ni datos de dibujo acá.
	const world::SpriteVisual* visual =
		content_.find(*entity.activeContent);
	if (visual == nullptr) {
		return;
	}

	const world::HeldOverlayFrame* frame =
		layout.frameAt(*overlay, phase, pass);
	if (frame == nullptr || !frame->drawn) {
		return;
	}

	// El contenido ocupa una capa 1x1 sin offset: la misma geometría
	// que las capas del overlay de herramientas (ancla, offsets, escala).
	const world::OverlayLayer layer{
		world::LayerFrame{world::Layer::Body, world::SheetCell{0, 0},
				  1, 1},
		0, 0};
	const world::PixelRect rect = layout.layerRect(*frame, layer);
	SpriteSheet& sheet = sheets_.sheet(
		visual->sheetImage, core::assetPath(visual->sheetImage),
		visual->cellWidth, visual->cellHeight);
	const SDL2pp::Point pivot(
		static_cast<int>(frame->pivotX * rect.width),
		static_cast<int>(frame->pivotY * rect.height));
	const Flip flip = frame->flip ? Flip::Horizontal : Flip::None;
	sheet.drawBlock(visual->col, visual->row, 1, 1,
			SDL2pp::Rect(rect.x, rect.y, rect.width, rect.height),
			flip, render::kFullSourceFraction,
			render::kFullSourceFraction, frame->angle, pivot);
}

}  // namespace sdl
