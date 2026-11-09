#pragma once

#include "world/HudOverlayCatalog.h"
#include "world/PixelBox.h"

namespace world {

// Geometría pura del relleno dinámico de un overlay: la cavidad interior
// (en px de la hoja, escalada) y la caja del sprite de relleno para una
// fracción 0..1, anclada abajo. No conoce SDL ni la fracción real: solo
// recibe el número ya resuelto por el estado.
class HudFillLayout {
public:
	// `shell`: caja final del overlay en pantalla. `scale`: la misma
	// escala con la que se dibuja la cáscara.
	HudFillLayout(const PixelBox& shell, const HudFill& fill, int scale);

	// Caja del relleno para una fracción 0..1. Con 0 no hay alto; con 1
	// ocupa toda la cavidad.
	PixelBox box(double fraction) const;

private:
	int x_;
	int y_;
	int width_;
	int height_;
};

}
