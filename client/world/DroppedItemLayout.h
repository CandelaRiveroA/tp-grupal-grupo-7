#pragma once

#include "world/ScenePixelMapper.h"
#include "world/SpriteVisual.h"

namespace world {

// Geometría pura del ítem caído en el piso: el rectángulo donde se
// dibuja (centrado en su celda y apoyado abajo). Una sola fuente para el
// pintor y el pick: lo que se clickea coincide siempre con lo que se
// ve. Sin SDL: solo rectángulos de mundo (§9).
class DroppedItemLayout {
public:
	// cellRect: la celda del ítem en px de mundo; visual: su sprite
	// (con su offset_y propio para apoyarlo en el piso).
	world::PixelRect rectFor(const world::PixelRect& cellRect,
				const world::SpriteVisual& visual) const;
};

}  // namespace world
