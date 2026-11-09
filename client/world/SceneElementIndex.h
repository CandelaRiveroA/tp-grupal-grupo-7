#pragma once

#include <vector>

#include "world/ChunkGrid.h"
#include "world/SceneDescription.h"
#include "world/ScenePixelMapper.h"

namespace world {

// Índice de los elementos estáticos de una escena. Se reconstruye cuando
// cambia la escena y se consulta con el rectángulo visible de la cámara.
class SceneElementIndex {
public:
	SceneElementIndex() = default;
	SceneElementIndex(const SceneElementIndex&) = delete;
	SceneElementIndex& operator=(const SceneElementIndex&) = delete;

	void rebuild(const SceneDescription& scene,
		     const ScenePixelMapper& mapper);
	void query(const PixelRect& visibleWorld,
		   std::vector<ChunkGrid::Index>& out) const;

private:
	ChunkGrid grid_;
};

}  // namespace world
