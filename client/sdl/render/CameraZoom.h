#pragma once

#include "world/ScenePixelMapper.h"

namespace sdl {

// Política de encuadre de la cámara: zoom de ajuste y offset de cada eje.
// Geometría pura (sin SDL, sin entidades, sin assets). Vive separada de
// Camera porque §11 pide aislar la lógica de zoom en su propio archivo.
class CameraZoom {
public:
	// Zoom que hace entrar el mundo en la ventana con margen
	// (kCameraFitRatio del eje más restrictivo), sin achicar nunca por
	// debajo del mínimo (los mapas grandes scrollean).
	double fitZoom(const world::PixelRect& worldBounds, int screenWidth,
		       int screenHeight) const;

	// Tamaño visible del mundo en un eje para un zoom dado.
	int visibleSize(int screenSize, double zoom) const;

	// Idem para el área de juego: la ventana sin la banda inferior del
	// HUD. Es donde se centra un mundo que entra entero, así el
	// inventario no le tapa la parte de abajo.
	int playVisibleSize(int screenSize, double zoom) const;

	// Offset de un eje: si el mundo entra, lo centra en el área de juego
	// (`playVisibleSize`); si no, sigue al target con el clamp de
	// siempre (`visibleSize`).
	int axisOffset(int targetCenter, int worldOrigin, int worldSize,
		       int visibleSize, int playVisibleSize) const;
};

}  // namespace sdl
