#pragma once

#include <vector>

#include "game/LocalWorldView.h"
#include "sdl/render/SheetCache.h"
#include "world/GroundCatalog.h"
#include "world/ScenePixelMapper.h"

namespace sdl {
class Viewport;
}

namespace sdl {

// Pinta el terreno con consecuencias (tierra tratada, mañana regado):
// por cada celda, la PIEZA que corresponde a sus vecinos del mismo
// parche (auto-tiling: world::CellPatch arma la máscara, el catálogo
// resuelve la pieza). Solo dibuja: QUÉ hay en cada celda lo dice la
// vista (lo mandó la fuente de estado) y CÓMO se ve, el catálogo. Los
// rectángulos salen de world::ScenePixelMapper y pasan por el viewport
// como todo dibujo de mundo (§9/§11).
class GroundEffectPainter {
public:
	GroundEffectPainter(SheetCache& sheets, const world::GroundCatalog& ground);

	// Dibuja la capa completa de efectos. El orden del vector es el de
	// aplicación: una consecuencia se dibuja encima de la anterior.
	void draw(const std::vector<game::ViewGroundEffect>& effects,
		  const world::ScenePixelMapper& mapper,
		  const Viewport& viewport) const;

private:
	SheetCache& sheets_;
	const world::GroundCatalog& ground_;
};

}
