#include "world/FurnitureGeometry.h"

#include "world/FurnitureLayout.h"

namespace world {

FurnitureGeometry::FurnitureGeometry(const ContentCatalog& content,
				     const NpcCatalog& props)
	: content_(content), props_(props), views_(content) {}

const FurnitureViews& FurnitureGeometry::views() const { return views_; }

const NpcDef* FurnitureGeometry::prop(const std::string& contentId) const {
	const NpcDef* def = props_.find(contentId);
	if (def == nullptr || def->hasPopup()) {
		return nullptr;
	}
	return def;
}

GridCell FurnitureGeometry::anchorOf(const std::string& contentId) const {
	const NpcDef* def = prop(contentId);
	return def != nullptr ? def->furnitureAnchor : GridCell{0, 0};
}

GridCell FurnitureGeometry::baseOf(const std::string& contentId,
				   const GridCell& source) const {
	const GridCell anchor = anchorOf(contentId);
	return GridCell{source.column - anchor.column, source.row + anchor.row};
}

GridCell FurnitureGeometry::sourceOf(const std::string& contentId,
				     const GridCell& base) const {
	const GridCell anchor = anchorOf(contentId);
	return GridCell{base.column + anchor.column, base.row - anchor.row};
}

int FurnitureGeometry::cellsFor(const int pixels, const int cellPixelSize) const {
	return (pixels + cellPixelSize - 1) / cellPixelSize;
}

std::optional<FurnitureShape> FurnitureGeometry::shapeAtBase(
	const std::string& contentId, const GridCell& base,
	const ScenePixelMapper& mapper, const FurnitureFacing facing) const {
	const int cell = mapper.cellPixelSize();
	if (const NpcDef* def = prop(contentId); def != nullptr) {
		// El tamaño del objeto por piezas sale de su cuerpo (drawn o
		// span * celda); la caja parte de la fila de arriba.
		const PixelRect size = def->body.box(cell, GridCell{0, 0});
		const GridSpan cells{cellsFor(size.width, cell),
				     cellsFor(size.height, cell)};
		const GridCell topLeft{base.column, base.row - cells.rows + 1};
		return FurnitureShape{base, topLeft, cells,
				      def->body.box(cell, topLeft)};
	}
	const FurnitureView view = views_.viewOf(contentId, facing);
	if (view.visual == nullptr) {
		return std::nullopt;
	}
	const SpriteVisual& visual = *view.visual;
	const GridCell topLeft{base.column, base.row - visual.rows + 1};
	return FurnitureShape{base,
			      topLeft,
			      GridSpan{visual.cols, visual.rows},
			      FurnitureLayout(mapper).box(visual, base),
			      view.visual,
			      view.mirrored};
}

std::optional<FurnitureShape> FurnitureGeometry::shapeOf(
	const std::string& contentId, const GridCell& source,
	const ScenePixelMapper& mapper, const FurnitureFacing facing) const {
	return shapeAtBase(contentId, baseOf(contentId, source), mapper, facing);
}

}
