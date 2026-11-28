#pragma once

#include "world/Grid.h"
#include "world/NpcCatalog.h"
#include "world/ScenePixelMapper.h"

namespace sdl {

class SheetCache;
class Viewport;

// Cómo pintar un objeto por piezas en este dibujo. DTO plano.
//   where:    celda arriba-izquierda del cuerpo.
//   frame:    cuadro de la animación (0 = cerrado / estático).
//   alpha:    opacidad 0..255 (la vista previa lo baja).
//   shift:    corrimiento en px de mundo (la vista previa se adelanta).
//   firstRow/lastRow: filas de la pieza a pintar (las frontales de la cama).
struct PropDraw {
	world::GridCell where;
	int frame = 0;
	int alpha = 255;
	world::PixelPoint shift{0, 0};
	int firstRow = 0;
	int lastRow = -1;  // -1 = hasta la última
};

// Pinta un objeto por piezas (cofre, cama) a partir de su definición de
// npcs.toml. Solo dibuja; QUÉ cuadro, DÓNDE y qué filas lo decide quien
// llama.
class PropPainter {
public:
	explicit PropPainter(SheetCache& sheets);

	void draw(const world::NpcDef& def, int cellPixelSize, const PropDraw& how,
		  const Viewport& viewport);

private:
	SheetCache& sheets_;
};

}
