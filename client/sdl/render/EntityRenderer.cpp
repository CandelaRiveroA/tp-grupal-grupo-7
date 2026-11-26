#include "sdl/render/EntityRenderer.h"

#include <SDL2pp/SDL2pp.hh>

#include "core/Constants.h"
#include "game/ActionResolver.h"
#include "game/LocalWorldView.h"
#include "game/ViewMotion.h"
#include "sdl/render/CharacterSprite.h"
#include "sdl/render/RenderConstants.h"
#include "sdl/render/SheetCache.h"
#include "sdl/render/SpriteSheet.h"
#include "sdl/render/Viewport.h"
#include "world/EntityAppearance.h"
#include "world/SceneDescription.h"

namespace sdl {

EntityRenderer::EntityRenderer(
	SdlContext& context, SheetCache& sheets,
	const world::CharacterCatalog& characters,
	const world::ToolOverlayCatalog& toolOverlay,
	const world::ContentCatalog& content,
	const world::CharacterPalettes& palettes)
	: sheets_(sheets)
	, characters_(characters)
	, appearancePainter_(context, sheets, characters, palettes)
	, toolOverlay_(sheets, toolOverlay, characters, content)
	, heldItem_(sheets, content, characters) {}

void EntityRenderer::draw(const game::ViewEntity& entity,
			  const std::size_t entityIndex,
			  const world::SceneDescription& scene,
			  const game::ViewMotion& motion,
			  const game::ActionResolver& actions,
			  const int animationTick,
			  const Viewport& viewport) const {
	const world::PixelRect box = viewport.toScreen(motion.drawBoxFor(
		entity, entityIndex, animationTick, scene.cellPixelSize));

	const world::PixelRect cullBox{
		box.x - render::kSpriteOverdrawMarginPixels,
		box.y - render::kSpriteOverdrawMarginPixels,
		box.width + render::kSpriteOverdrawMarginPixels * 2,
		box.height + render::kSpriteOverdrawMarginPixels * 2,
	};
	if (!viewport.intersects(cullBox)) {
		return;
	}
	const world::CreatureData* creature = characters_.find(entity.kind);
	if (creature == nullptr) {
		return;
	}

	const int phase =
		actions.phaseFor(entity, entityIndex, animationTick);
	const world::ActionKind action =
		actions.actionFor(entity, entityIndex, animationTick);

	const world::AppearanceResolver resolver(*creature);
	const world::EntityAppearance appearance =
		resolver.appearanceFor(entity.facing, action, phase);

	const Flip flip =
		appearance.mirrored ? Flip::Horizontal : Flip::None;
	const SDL2pp::Rect drawBox(box.x, box.y, box.width, box.height);

	// El overlay se dibuja en DOS pasadas alrededor del cuerpo: los
	// frames con `behind` (la espada de espaldas) detrás —el cuerpo los
	// tapa donde se superponen— y el resto delante (caso histórico).
	toolOverlay_.draw(entity, drawBox, phase, action, *appearance.pose,
			  world::OverlayPass::Behind);
	if (characters_.hasAppearance(entity.kind)) {
		// Apariencia por capas (ropa/pelo): la dibuja su pintor.
		const int frame =
			resolver.frameIndexFor(entity.facing, action, phase);
		appearancePainter_.draw(entity, *appearance.pose, action,
					frame, drawBox, flip);
	} else {
		// La acción puede traer su PROPIA hoja (enemigos: una por
		// acción); si no la declara, se usa la de la criatura (el
		// jugador).
		const world::ActionData& actionData = *appearance.action;
		const bool ownSheet = !actionData.sheet.empty();
		const std::string& sheetFile =
			ownSheet ? actionData.sheet
				 : characters_.sheetFile(entity.kind);
		const int cellW = ownSheet ? actionData.cellWidth
					   : characters_.sheetCellWidth(entity.kind);
		const int cellH = ownSheet ? actionData.cellHeight
					   : characters_.sheetCellHeight(entity.kind);
		const int frameCols = ownSheet ? actionData.frameCols
					       : characters_.frameCols(entity.kind);
		const int frameRows = ownSheet ? actionData.frameRows
					       : characters_.frameRows(entity.kind);
		SpriteSheet& sheet =
			sheets_.sheet(sheetFile, core::assetPath(sheetFile),
				      cellW, cellH);
		CharacterSprite sprite(
			sheet, characters_.drawnWidth(entity.kind),
			characters_.drawnHeight(entity.kind), frameCols,
			frameRows);
		sprite.draw(*appearance.pose, drawBox, flip);
	}
	toolOverlay_.draw(entity, drawBox, phase, action, *appearance.pose,
			  world::OverlayPass::Front);

	// El contenido sostenido (poses "sosteniendo", una por variante de
	// carry) va sobre la cabeza: el MISMO sprite que el del inventario.
	// Qué se sostiene y en qué pose lo resolvió game::ActionResolver;
	// esto solo lo dibuja.
	if (actions.isHoldingPose(action) &&
	    actions.heldContentId().has_value()) {
		heldItem_.draw(*actions.heldContentId(), entity.kind, drawBox);
	}
}

const PlayerAppearancePainter& EntityRenderer::appearancePainter() const {
	return appearancePainter_;
}

}  // namespace sdl
