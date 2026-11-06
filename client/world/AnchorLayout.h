#pragma once

#include <map>

#include "world/PixelBox.h"

namespace world {

// Vocabulario de anclajes a los bordes/centro de la ventana. Es la grilla
// 3x3: eje horizontal (izquierda/centro/derecha) x eje vertical
// (arriba/centro/abajo).
enum class Anchor {
	TopLeft,
	TopCenter,
	TopRight,
	CenterLeft,
	Center,
	CenterRight,
	BottomLeft,
	BottomCenter,
	BottomRight,
};

// Geometría PURA de anclaje: dónde cae una caja de tamaño dado dentro de
// una ventana, según un Anchor y un margen por eje. Sin SDL. Es EL lugar
// único de la cuenta "alinear contra un borde": el HUD de overlays, la
// barra del inventario y la pila de banners delegan acá en vez de cada uno
// reimplementar la aritmética (convención §9 "buscar lo genérico").
//
// El margen es distancia desde el borde al que se ancla (para las
// alineaciones al centro, el margen desplaza desde el centro).
class AnchorLayout {
public:
	AnchorLayout();

	// Caja posicionada: origen según anchor + márgenes, del tamaño pedido.
	PixelBox box(Anchor anchor, int marginX, int marginY, int width,
		     int height, int windowWidth, int windowHeight) const;

private:
	// Posición en un eje: desde el inicio (leading), desde el centro
	// (centered) o desde el final (trailing). leading == top en el eje
	// vertical y trailing == bottom: por eso una sola fórmula por caso.
	int leading(int windowLen, int boxLen, int margin) const;
	int centered(int windowLen, int boxLen, int margin) const;
	int trailing(int windowLen, int boxLen, int margin) const;

	// Cómo se resuelve cada eje para cada Anchor. Tabla miembro (§9:
	// mapa sobre switch; mismo patrón que ToolOverlayLayout::anchorBases_).
	using AxisPos = int (AnchorLayout::*)(int windowLen, int boxLen,
					      int margin) const;
	struct AxisOps {
		AxisPos x;
		AxisPos y;
	};
	std::map<Anchor, AxisOps> ops_;
};

}
