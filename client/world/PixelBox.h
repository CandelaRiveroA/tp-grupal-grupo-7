#pragma once

namespace world {

// Caja en píxeles de pantalla. Es el tipo común de "dónde cae algo" en
// toda la geometría pura (inventario, banners, overlays de HUD). Vive en
// su propio header para que las capas de geometría lo usen sin arrastrar
// el layout que los contiene (evita dependencias circulares).
struct PixelSize {
	int width = 0;
	int height = 0;
};

struct PixelBox {
	int x = 0;
	int y = 0;
	int width = 0;
	int height = 0;

	bool contains(int px, int py) const;
	PixelBox inset(int px) const;
	PixelBox outset(int px) const;
	PixelBox fitCentered(const PixelSize& content) const;
};

}
