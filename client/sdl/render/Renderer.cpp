#include "sdl/render/Renderer.h"

#include "core/Constants.h"
#include "game/ActionResolver.h"
#include "sdl/context/SdlContext.h"
#include "sdl/render/ElementPainter.h"
#include "sdl/render/RenderConstants.h"
#include "sdl/render/Viewport.h"
#include "world/DroppedItemLayout.h"
#include "world/FurnitureLayout.h"
#include "world/ScenePixelMapper.h"
#include "world/WorldConstants.h"

namespace sdl {

Renderer::Renderer(SdlContext& context,
		   const world::CharacterCatalog& characters,
		   const world::ToolOverlayCatalog& toolOverlay,
		   const world::ResourceCatalog& resources,
		   const world::ContentCatalog& content,
		   const world::GroundCatalog& ground,
		   const world::CropCatalog& crops,
		   const world::CharacterPalettes& palettes)
	: context_(context)
	, resources_(resources)
	, content_(content)
	, ground_(ground)
	, sheetCache_(context)
	, entityRenderer_(context, sheetCache_, characters, toolOverlay,
			  content, palettes)
	, layerPainter_(sheetCache_)
	, npcRenderer_(context)
	, groundPainter_(sheetCache_, ground)
	, cropPainter_(sheetCache_, crops) {}

void Renderer::prepareScene(const world::SceneDescription& scene) {
	const world::ScenePixelMapper mapper(scene.cellPixelSize);
	sceneIndex_.rebuild(scene, mapper);
	sceneIndexReady_ = true;
}

void Renderer::setNpcs(const std::vector<const world::NpcDef*>& npcs) {
	npcRenderer_.setNpcs(npcs);
}

void Renderer::setNpcFrames(const std::map<std::string, int>& frames) {
	npcRenderer_.setFrames(frames);
}

void Renderer::drawWorld(const world::SceneDescription& scene,
			 const game::LocalWorldView& view,
			 const game::ViewMotion& motion,
			 const game::ActionResolver& actionResolver,
			 const int animationTick,
			 const Viewport& viewport,
			 const world::TimeOfDay timeOfDay) {
	const world::Color& background = scene.floorColor;
	context_.clear(SDL2pp::Color(background.r, background.g,
				     background.b, background.a));

	if (!sceneIndexReady_) {
		prepareScene(scene);
	}

	const world::ScenePixelMapper mapper(scene.cellPixelSize);
	const world::PixelRect visibleWorld = viewport.visibleWorldRect();
	const world::PixelRect dynamicQuery{
		visibleWorld.x - render::kSpriteOverdrawMarginPixels,
		visibleWorld.y - render::kSpriteOverdrawMarginPixels,
		visibleWorld.width + render::kSpriteOverdrawMarginPixels * 2,
		visibleWorld.height + render::kSpriteOverdrawMarginPixels * 2,
	};

	viewIndex_.rebuild(view, mapper);

	sceneIndex_.query(visibleWorld, visibleElements_);
	viewIndex_.queryResources(dynamicQuery, visibleResources_);
	viewIndex_.queryDroppedItems(dynamicQuery, visibleItems_);
	viewIndex_.queryEntities(dynamicQuery, visibleEntities_);

	// El piso en grilla (mapas del editor) va debajo de todo elemento.
	layerPainter_.draw(scene, mapper, viewport);

	depth_.clear();
	for (const std::uint32_t index : visibleElements_) {
		const world::ElementPlacement& element = scene.elements[index];
		// El momento del día filtra: lo que solo se dibuja en su
		// momento (key `when` de scenes.toml) no pasa de acá; el
		// índice espacial y el resto del frame no se enteran.
		if (!element.visibleAt(timeOfDay)) {
			continue;
		}
		// Lo que está "de pie" (árboles, edificios) se ordena con las
		// entidades más abajo; el resto va ya, debajo de todo.
		if (element.depthSorted) {
			depth_.addElement(element.at.row + element.span.rows - 1,
					  index);
			continue;
		}
		drawElement(scene, element, mapper, viewport);
	}

	// El terreno con consecuencias (tierra tratada por la pala, regada
	// por la regadera): cada celda dibuja la pieza que arma su zona
	// (auto-tiling por vecinos). QUÉ hay en cada celda lo dice la
	// vista; esto solo lo pinta.
	groundPainter_.draw(view.groundEffects, mapper, viewport);

	// Los cultivos plantados (el crecimiento que mandó la fuente):
	// encima del terreno de su celda y debajo de las entidades.
	cropPainter_.draw(view.plantedCrops, mapper, viewport);

	// Recursos antes que las entidades: el personaje se dibuja encima.
	for (const std::uint32_t index : visibleResources_) {
		drawResource(view.resources[index], mapper, viewport);
	}

	// Minerales caídos al piso: también debajo de las entidades, para que
	// el personaje pase por encima al pararse sobre ellos.
	for (const std::uint32_t index : visibleItems_) {
		drawDroppedItem(view.droppedItems[index], mapper, viewport);
	}

	// NPCs estáticos: debajo de las entidades, como los recursos.
	npcRenderer_.draw(viewport, scene.cellPixelSize,
			  npcPlacements_.cellsFrom(view, scene));

	// Entidades y elementos de pie, en orden de profundidad: quien está
	// detrás de la base de un árbol queda tapado por él.
	for (std::size_t i = 0; i < view.placedFurniture.size(); ++i) {
		depth_.addFurniture(view.placedFurniture[i].cellY,
				    static_cast<std::uint32_t>(i));
	}
	for (const std::uint32_t index : visibleEntities_) {
		depth_.addEntity(view.entities[index].cellY, index);
	}
	for (const world::DepthItem& item : depth_.sorted()) {
		if (item.kind == world::DepthKind::Element) {
			drawElement(scene, scene.elements[item.index], mapper,
				    viewport);
			continue;
		}
		if (item.kind == world::DepthKind::Furniture) {
			drawFurniture(view.placedFurniture[item.index], mapper,
				      viewport);
			continue;
		}
		entityRenderer_.draw(view.entities[item.index], item.index, scene,
				    motion, actionResolver, animationTick,
				    viewport);
	}
}

void Renderer::drawElement(const world::SceneDescription& scene,
			   const world::ElementPlacement& element,
			   const world::ScenePixelMapper& mapper,
			   const Viewport& viewport) {
	// El pase normal: la pieza entera del elemento.
	drawElementRange(scene, element, mapper, viewport, 0,
			 element.pieceSize.rows - 1);
}

void Renderer::drawElementFront(const world::SceneDescription& scene,
				const world::ElementPlacement& element,
				const world::ScenePixelMapper& mapper,
				const Viewport& viewport, const int firstRow,
				const int lastRow) {
	drawElementRange(scene, element, mapper, viewport, firstRow,
			 lastRow);
}

void Renderer::drawElementRange(const world::SceneDescription& scene,
				const world::ElementPlacement& element,
				const world::ScenePixelMapper& mapper,
				const Viewport& viewport, const int firstRow,
				const int lastRow) {
	const world::TileSetDef& tileset =
		scene.tilesets.at(element.tileset);

	SpriteSheet& sheet = sheetCache_.sheet(
		element.tileset, core::assetPath(tileset.image),
		tileset.cellWidth, tileset.cellHeight);

	const world::PixelRect area =
		viewport.toScreen(mapper.toPixels(element.at, element.span));

	const ElementPainter painter(tileset, element, area);
	if (!viewport.intersects(painter.bounds())) {
		return;
	}
	painter.paint(sheet, viewport.screenRect(), firstRow, lastRow);
}

void Renderer::drawResource(const game::ViewResource& resource,
			    const world::ScenePixelMapper& mapper,
			    const Viewport& viewport) {
	const world::ResourceVisual* found =
		resources_.find(resource.contentId, resource.state);
	if (found == nullptr) {
		return;
	}
	const world::ResourceVisual& visual = *found;

	const world::PixelRect cell = viewport.toScreen(mapper.toPixels(
		world::GridCell{resource.cellX, resource.cellY},
		world::kUnitCellSpan));

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

	const world::PixelRect drawRect{x, y, w, h};
	if (!viewport.intersects(drawRect)) {
		return;
	}

	SpriteSheet& sheet = sheetCache_.sheet(
		visual.sprite.sheetImage,
		core::assetPath(visual.sprite.sheetImage),
		visual.sprite.cellWidth, visual.sprite.cellHeight);

	sheet.drawBlock(visual.sprite.col, visual.sprite.row, visual.cols,
			visual.rows,
			SDL2pp::Rect(x, y, w, h));
}

void Renderer::drawFurniture(const game::ViewFurniture& furniture,
			     const world::ScenePixelMapper& mapper,
			     const Viewport& viewport) {
	// Un contentId sin visual no se dibuja (igual que un ítem del piso).
	const world::SpriteVisual* visual = content_.find(furniture.contentId);
	if (visual == nullptr) {
		return;
	}
	const world::PixelRect box = world::FurnitureLayout(mapper).box(
		*visual, world::GridCell{furniture.cellX, furniture.cellY});
	const world::PixelRect onScreen = viewport.toScreen(box);
	if (!viewport.intersects(onScreen)) {
		return;
	}
	SpriteSheet& sheet = sheetCache_.sheet(
		visual->sheetImage, core::assetPath(visual->sheetImage),
		visual->cellWidth, visual->cellHeight);
	sheet.drawBlock(visual->col, visual->row, visual->cols, visual->rows,
			SDL2pp::Rect(onScreen.x, onScreen.y, onScreen.width,
				     onScreen.height));
}

void Renderer::drawDroppedItem(const game::ViewDroppedItem& item,
			       const world::ScenePixelMapper& mapper,
			       const Viewport& viewport) {
	// El item en el piso se dibuja con el MISMO asset que en el slot del
	// inventario (ContentCatalog, por contentId). Así un mineral, un
	// huevo o cualquier recolectable futuro usan su celda sin casos
	// especiales. Más chico que la celda y centrado, para que se lea
	// como un item apoyado en el suelo.
	const world::SpriteVisual* found = content_.find(item.contentId);
	if (found == nullptr) {
		return;
	}
	const world::SpriteVisual& visual = *found;

	// La geometría del ítem la calcula DroppedItemLayout: la misma que
	// usa el pick, así lo que se toca coincide siempre con lo que se ve.
	// El ajuste vertical para el piso lo lleva el sprite (offset_y).
	const world::DroppedItemLayout layout;
	const world::PixelRect drawRect = viewport.toScreen(layout.rectFor(
		mapper.toPixels(world::GridCell{item.cellX, item.cellY},
				world::kUnitCellSpan),
		visual));
	if (!viewport.intersects(drawRect)) {
		return;
	}

	SpriteSheet& sheet = sheetCache_.sheet(
		visual.sheetImage, core::assetPath(visual.sheetImage),
		visual.cellWidth, visual.cellHeight);

	sheet.draw(visual.col, visual.row,
		   SDL2pp::Rect(drawRect.x, drawRect.y, drawRect.width,
				drawRect.height));
}

}  // namespace sdl
