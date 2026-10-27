#include "Renderer.h"

#include <map>

#include "core/Constants.h"
#include "game/ActionResolver.h"
#include "sdl/context/SdlConstants.h"
#include "sdl/context/SdlContext.h"
#include "sdl/render/ElementPainter.h"
#include "world/CharacterAppearance.h"
#include "world/EntityAppearance.h"
#include "world/ScenePixelMapper.h"

namespace sdl {

Renderer::Renderer(SdlContext& context,
		   const world::CharacterCatalog& characters,
		   const world::ToolOverlayCatalog& toolOverlay,
		   const world::ResourceCatalog& resources)
	: context_(context)
	, catalog_(characters)
	, toolOverlay_(toolOverlay)
	, resources_(resources)
	, characterSheet_(context,
			  core::assetPath(characters.sheetFile(
				  world::CreatureKind::Player)),
			  characters.sheetCellWidth(world::CreatureKind::Player),
			  characters.sheetCellHeight(world::CreatureKind::Player))
	, character_(characterSheet_,
		     characters.drawnWidth(world::CreatureKind::Player),
		     characters.drawnHeight(world::CreatureKind::Player))
	, sheetCache_(context) {}

void Renderer::render(const world::SceneDescription& scene,
		      const game::LocalWorldView& view,
		      const game::ViewMotion& motion,
		      const game::ActionResolver& actionResolver,
		      int animationTick) {
	renderScene(scene, view, motion, actionResolver, animationTick);
	context_.present();
}

void Renderer::renderScene(const world::SceneDescription& scene,
			   const game::LocalWorldView& view,
			   const game::ViewMotion& motion,
			   const game::ActionResolver& actionResolver,
			   int animationTick) {

	const world::Color& background = scene.floorColor;
	context_.clear(SDL2pp::Color(background.r, background.g,
				     background.b, background.a));

	for (const world::ElementPlacement& element : scene.elements) {
		drawElement(scene, element);
	}

	// Recursos antes que las entidades: el personaje se dibuja encima.
	for (const game::ViewResource& resource : view.resources) {
		drawResource(scene, resource);
	}

	for (std::size_t i = 0; i < view.entities.size(); ++i) {
		drawEntity(scene, view.entities[i], motion, actionResolver, i,
			   animationTick);
	}
}

void Renderer::drawEntity(const world::SceneDescription& scene,
			  const game::ViewEntity& entity,
			  const game::ViewMotion& motion,
			  const game::ActionResolver& actionResolver,
			  std::size_t entityIndex, int animationTick) {
	const world::ScenePixelMapper mapper(scene.cellPixelSize);
	const world::PixelRect box =
		mapper.toPixels(world::GridCell{entity.cellX, entity.cellY},
				world::GridSpan{1, 1});

	const int phase =
		actionResolver.phaseFor(entity, entityIndex, animationTick);

	const world::ActionKind action =
		actionResolver.actionFor(entity, entityIndex, animationTick);

	const world::AppearanceResolver resolver(catalog_.dataFor(entity.kind));
	const world::EntityAppearance appearance =
		resolver.appearanceFor(entity.facing, action, phase);

	const game::PixelOffset offset = motion.offsetFor(
		entityIndex, animationTick, scene.cellPixelSize);

	const Flip flip =
		appearance.mirrored ? Flip::Horizontal : Flip::None;

	const SDL2pp::Rect drawBox(box.x + offset.x, box.y + offset.y,
				   box.width, box.height);

	character_.draw(*appearance.pose, drawBox, flip);

	drawToolOverlay(entity, drawBox, phase, action);
}

void Renderer::drawToolOverlay(const game::ViewEntity& entity,
			       const SDL2pp::Rect& drawBox, int phase,
			       world::ActionKind action) {
	if (action != world::ActionKind::UseTool) {
		return;
	}
	if (!entity.activeTool.has_value()) {
		return;
	}
	const world::ItemKind tool = *entity.activeTool;
	const world::Direction dir = entity.facing;
	if (!toolOverlay_.has(tool, dir)) {
		return;
	}
	const world::ToolOverlayAction& overlayAction =
		toolOverlay_.actionFor(tool, dir);
	if (overlayAction.frames.empty()) {
		return;
	}

	const int frameCount =
		static_cast<int>(overlayAction.frames.size());
	const int frameIdx = (overlayAction.frameTicks > 0)
		? (phase / overlayAction.frameTicks)
		: 0;
	const int clampedIdx =
		frameIdx < frameCount ? frameIdx : frameCount - 1;
	const world::ToolOverlayFrame& frame =
		overlayAction.frames[static_cast<std::size_t>(clampedIdx)];

	SpriteSheet& sheet = sheetCache_.sheet(
		overlayAction.sheetImage,
		core::assetPath(overlayAction.sheetImage),
		overlayAction.cellWidth, overlayAction.cellHeight);

	// Alineado al cuerpo con el mismo drawnWidth/Height que CharacterSprite.
	const int drawnW = catalog_.drawnWidth(entity.kind);
	const int drawnH = catalog_.drawnHeight(entity.kind);
	const int bodyX  = drawBox.x + (drawBox.w - drawnW) / 2;
	const int bodyBottom = drawBox.y + drawBox.h;
	const int bodyTop = bodyBottom - drawnH;

	for (const world::LayerFrame& layer : frame.layers) {
		const int pxPerCellX = drawnW / layer.cols;
		const int pxPerCellY = drawnH / layer.rows;
		const int w = static_cast<int>(layer.cols * pxPerCellX *
					       frame.scale);
		const int h = static_cast<int>(layer.rows * pxPerCellY *
					       frame.scale);
		const int x = bodyX + frame.offsetX;
		// Diccionario (no switch, por contrato) de anclaje a y base.
		const std::map<world::FrameAnchor, int> anchorY = {
			{world::FrameAnchor::Top, bodyTop - h},
			{world::FrameAnchor::Body, bodyTop},
			{world::FrameAnchor::Bottom, bodyBottom - h},
		};
		const int y = anchorY.at(frame.anchor) +
			      toolOverlay_.offsetFor(frame.anchor) +
			      frame.offsetY;
		// Pivote en píxeles del rect: gira sobre el mango, no el centro.
		const SDL2pp::Point pivot(
			static_cast<int>(frame.pivotX * w),
			static_cast<int>(frame.pivotY * h));
		const Flip flip =
			frame.flip ? Flip::Horizontal : Flip::None;
		sheet.drawBlock(layer.frame.col, layer.frame.row,
				layer.cols, layer.rows,
				SDL2pp::Rect(x, y, w, h), flip, 1.0,
				1.0, frame.angle, pivot);
	}
}

void Renderer::drawElement(const world::SceneDescription& scene,
			   const world::ElementPlacement& element) {

	const world::TileSetDef& tileset =
		scene.tilesets.at(element.tileset);

	SpriteSheet& sheet = sheetCache_.sheet(
		element.tileset, core::assetPath(tileset.image),
		tileset.cellWidth, tileset.cellHeight);

	const world::ScenePixelMapper mapper(scene.cellPixelSize);
	const world::PixelRect area =
		mapper.toPixels(element.at, element.span);

	ElementPainter(tileset, element, area).paint(sheet);
}

void Renderer::drawResource(const world::SceneDescription& scene,
			    const game::ViewResource& resource) {
	if (!resources_.has(resource.kind, resource.state)) {
		return;
	}
	const world::ResourceVisual& visual =
		resources_.visualFor(resource.kind, resource.state);

	SpriteSheet& sheet = sheetCache_.sheet(
		visual.sheetImage, core::assetPath(visual.sheetImage),
		visual.cellWidth, visual.cellHeight);

	const world::ScenePixelMapper mapper(scene.cellPixelSize);
	const world::PixelRect cell = mapper.toPixels(
		world::GridCell{resource.cellX, resource.cellY},
		world::GridSpan{1, 1});

	// Anclado a la base de la celda: al achicarse se desmorona desde
	// arriba. scale agranda para tapar el aire transparente de arriba.
	const int baseW = cell.width;
	const int baseH = (visual.cols > 0)
		? baseW * visual.rows / visual.cols
		: cell.height;
	const int w = static_cast<int>(baseW * resources_.scale);
	const int h = static_cast<int>(baseH * resources_.scale);
	const int x = cell.x + (cell.width - w) / 2;
	const int y = cell.y + cell.height - h + resources_.offsetY;

	sheet.drawBlock(visual.col, visual.row, visual.cols, visual.rows,
			SDL2pp::Rect(x, y, w, h));
}

}
