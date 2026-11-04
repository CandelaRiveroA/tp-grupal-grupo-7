#pragma once

#include <vector>

#include "game/LocalWorldView.h"
#include "sdl/render/SheetCache.h"
#include "world/CropCatalog.h"
#include "world/ScenePixelMapper.h"

namespace sdl {
class Viewport;
}

namespace sdl {

// Pinta los cultivos plantados: por cada celda, el sprite de su etapa
// de crecimiento (crops.toml). Solo dibuja: QUÉ hay en cada celda lo
// dice la vista (lo mandó la fuente de estado) y CÓMO se ve, el
// catálogo. Los rectángulos salen de world::ScenePixelMapper y pasan
// por el viewport como todo dibujo de mundo (§9/§11).
class CropPainter {
public:
	CropPainter(SheetCache& sheets, const world::CropCatalog& crops);

	void draw(const std::vector<game::ViewCrop>& planted,
		  const world::ScenePixelMapper& mapper,
		  const Viewport& viewport) const;

private:
	SheetCache& sheets_;
	const world::CropCatalog& crops_;
};

}  // namespace sdl
