#pragma once

#include "world/CharacterLayout.h"
#include "world/ScenePixelMapper.h"

namespace world {

// Geometría pura del contenido sostenido sobre la cabeza (pose Carry).
// No conoce SDL ni assets: a partir del box dibujado del cuerpo calcula
// el rectángulo del ítem, centrado horizontalmente sobre la cabeza y
// a `offsetY` px bajo su borde superior. El pintado queda en
// sdl::HeldItemPainter (convención §9: geometría separada del pintado).
class HeldItemLayout {
public:
	HeldItemLayout(const PixelRect& bodyBox, int drawnWidth,
		       int drawnHeight, int itemWidth, int itemHeight,
		       int offsetY);

	PixelRect rect() const;

private:
	// El cuerpo del pose: de ahí salen bodyX/bodyTop (la misma
	// aritmética que usan CharacterSprite y ToolOverlayLayout).
	CharacterLayout body_;
	int itemWidth_;
	int itemHeight_;
	int offsetY_;
};

}
