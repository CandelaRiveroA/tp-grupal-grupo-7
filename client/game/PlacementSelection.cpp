#include "game/PlacementSelection.h"

#include "game/GameConstants.h"
#include "world/FurnitureFootprint.h"

namespace game {

PlacementSelection::PlacementSelection(const world::FurnitureGeometry& furniture)
	: furniture_(furniture) {}

void PlacementSelection::observe(const LocalWorldView& view,
				 const CarrySelection& carry,
				 const world::GridCell& hovered,
				 const world::GridSpan& sceneGrid,
				 const world::FurnitureFacing facing) {
	spot_.reset();
	if (!carry.held().has_value() || !carry.heldFootprint().has_value()) {
		return;
	}
	if (!isReachable(view, hovered)) {
		return;
	}
	PlacementSpot spot;
	spot.contentId = *carry.held();
	spot.base = hovered;
	spot.source = furniture_.sourceOf(spot.contentId, hovered);
	const ViewEntity& player = view.entities[kLocalPlayerIndex];
	spot.toward = world::GridCell{hovered.column - player.cellX,
				      hovered.row - player.cellY};
	spot.facing = facing;
	spot.cells = world::FurnitureFootprint(
			     furniture_.views().footprintOf(*carry.heldFootprint(), facing))
			     .cellsAt(hovered);
	spot.valid = isFree(spot.cells, occupiedCells(view), sceneGrid);
	spot_ = spot;
}

const std::optional<PlacementSpot>& PlacementSelection::spot() const {
	return spot_;
}

bool PlacementSelection::isReachable(const LocalWorldView& view,
				     const world::GridCell& hovered) const {
	if (view.entities.empty()) {
		return false;
	}
	const ViewEntity& player = view.entities[kLocalPlayerIndex];
	const int dx = hovered.column - player.cellX;
	const int dy = hovered.row - player.cellY;
	return (dx == 0 && (dy == 1 || dy == -1)) ||
	       (dy == 0 && (dx == 1 || dx == -1));
}

std::set<PlacementSelection::CellKey> PlacementSelection::occupiedCells(
	const LocalWorldView& view) const {
	std::set<CellKey> occupied;
	for (const ViewEntity& entity : view.entities) {
		occupied.insert({entity.cellX, entity.cellY});
	}
	for (const ViewResource& resource : view.resources) {
		occupied.insert({resource.cellX, resource.cellY});
	}
	for (const ViewDroppedItem& item : view.droppedItems) {
		occupied.insert({item.cellX, item.cellY});
	}
	for (const ViewCrop& crop : view.plantedCrops) {
		occupied.insert({crop.cellX, crop.cellY});
	}
	for (const ViewFurniture& furniture : view.placedFurniture) {
		const world::FurnitureFootprint footprint(
			furniture_.views().footprintOf(furniture.footprint, furniture.facing));
		for (const world::GridCell& cell : footprint.cellsAt(furniture_.baseOf(
			     furniture.contentId,
			     world::GridCell{furniture.cellX, furniture.cellY}))) {
			occupied.insert({cell.column, cell.row});
		}
	}
	return occupied;
}

bool PlacementSelection::isFree(const std::vector<world::GridCell>& cells,
				const std::set<CellKey>& occupied,
				const world::GridSpan& sceneGrid) const {
	for (const world::GridCell& cell : cells) {
		const bool inside = cell.column >= 0 && cell.row >= 0 &&
				    cell.column < sceneGrid.columns &&
				    cell.row < sceneGrid.rows;
		if (!inside || occupied.count({cell.column, cell.row}) > 0) {
			return false;
		}
	}
	return true;
}

}
