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
	SheetCache& sheets, const world::CharacterCatalog& characters,
	const world::ToolOverlayCatalog& toolOverlay)
	: sheets_(sheets)
	, characters_(characters)
	, toolOverlay_(sheets, toolOverlay, characters) {}

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

	SpriteSheet& sheet = sheets_.sheet(
		characters_.sheetFile(entity.kind),
		core::assetPath(characters_.sheetFile(entity.kind)),
		characters_.sheetCellWidth(entity.kind),
		characters_.sheetCellHeight(entity.kind));
	CharacterSprite sprite(sheet, characters_.drawnWidth(entity.kind),
			      characters_.drawnHeight(entity.kind));

	const Flip flip =
		appearance.mirrored ? Flip::Horizontal : Flip::None;
	const SDL2pp::Rect drawBox(box.x, box.y, box.width, box.height);
	sprite.draw(*appearance.pose, drawBox, flip);
	toolOverlay_.draw(entity, drawBox, phase, action);
}

}  // namespace sdl
