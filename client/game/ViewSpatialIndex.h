#pragma once

#include <vector>

#include "game/LocalWorldView.h"
#include "world/ChunkGrid.h"
#include "world/ScenePixelMapper.h"

namespace game {

// Índice dinámico del snapshot actual. Se reconstruye una vez por frame:
// las entidades pueden cambiar de celda y los recursos/items aparecer o
// desaparecer. No guarda ids ni estado de juego.
class ViewSpatialIndex {
public:
	ViewSpatialIndex() = default;
	ViewSpatialIndex(const ViewSpatialIndex&) = delete;
	ViewSpatialIndex& operator=(const ViewSpatialIndex&) = delete;

	void rebuild(const LocalWorldView& view,
		     const world::ScenePixelMapper& mapper);

	void queryResources(const world::PixelRect& visibleWorld,
			    std::vector<world::ChunkGrid::Index>& out) const;
	void queryDroppedItems(const world::PixelRect& visibleWorld,
			       std::vector<world::ChunkGrid::Index>& out) const;
	void queryEntities(const world::PixelRect& visibleWorld,
			   std::vector<world::ChunkGrid::Index>& out) const;

private:
	world::ChunkGrid resources_;
	world::ChunkGrid droppedItems_;
	world::ChunkGrid entities_;
};

}  // namespace game
