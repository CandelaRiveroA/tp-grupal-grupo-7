#include "game/ViewSpatialIndex.h"

#include "world/WorldConstants.h"

namespace game {

void ViewSpatialIndex::rebuild(const LocalWorldView& view,
			       const world::ScenePixelMapper& mapper) {
	resources_.setChunkSize(mapper.cellPixelSize());
	droppedItems_.setChunkSize(mapper.cellPixelSize());
	entities_.setChunkSize(mapper.cellPixelSize());

	resources_.clear();
	droppedItems_.clear();
	entities_.clear();

	for (std::size_t i = 0; i < view.resources.size(); ++i) {
		const ViewResource& resource = view.resources[i];
		resources_.insert(
			static_cast<world::ChunkGrid::Index>(i),
			mapper.toPixels(world::GridCell{resource.cellX,
							resource.cellY},
					world::kUnitCellSpan));
	}

	for (std::size_t i = 0; i < view.droppedItems.size(); ++i) {
		const ViewDroppedItem& item = view.droppedItems[i];
		droppedItems_.insert(
			static_cast<world::ChunkGrid::Index>(i),
			mapper.toPixels(world::GridCell{item.cellX, item.cellY},
					world::kUnitCellSpan));
	}

	for (std::size_t i = 0; i < view.entities.size(); ++i) {
		const ViewEntity& entity = view.entities[i];
		entities_.insert(
			static_cast<world::ChunkGrid::Index>(i),
			mapper.toPixels(world::GridCell{entity.cellX,
							entity.cellY},
					world::kUnitCellSpan));
	}
}

void ViewSpatialIndex::queryResources(
	const world::PixelRect& visibleWorld,
	std::vector<world::ChunkGrid::Index>& out) const {
	resources_.query(visibleWorld, out);
}

void ViewSpatialIndex::queryDroppedItems(
	const world::PixelRect& visibleWorld,
	std::vector<world::ChunkGrid::Index>& out) const {
	droppedItems_.query(visibleWorld, out);
}

void ViewSpatialIndex::queryEntities(
	const world::PixelRect& visibleWorld,
	std::vector<world::ChunkGrid::Index>& out) const {
	entities_.query(visibleWorld, out);
}

}  // namespace game
