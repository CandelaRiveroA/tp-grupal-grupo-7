#include "world/SceneElementIndex.h"

#include "world/ElementLayout.h"

namespace world {

void SceneElementIndex::rebuild(const SceneDescription& scene,
				const ScenePixelMapper& mapper) {
	grid_.setChunkSize(mapper.cellPixelSize());
	grid_.clear();
	for (std::size_t i = 0; i < scene.elements.size(); ++i) {
		const ElementPlacement& element = scene.elements[i];
		const auto tileset = scene.tilesets.find(element.tileset);
		if (tileset == scene.tilesets.end()) {
			continue;
		}

		const PixelRect area =
			mapper.toPixels(element.at, element.span);
		const ElementLayout layout(tileset->second, element, area);
		grid_.insert(static_cast<ChunkGrid::Index>(i), layout.bounds());
	}
}

void SceneElementIndex::query(const PixelRect& visibleWorld,
			      std::vector<ChunkGrid::Index>& out) const {
	grid_.query(visibleWorld, out);
}

}  // namespace world
