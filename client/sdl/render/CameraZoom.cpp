#include "sdl/render/CameraZoom.h"

#include <algorithm>
#include <cmath>

#include "sdl/render/RenderConstants.h"

namespace sdl {

double CameraZoom::fitZoom(const world::PixelRect& worldBounds,
			   const int screenWidth,
			   const int screenHeight) const {
	if (worldBounds.width <= 0 || worldBounds.height <= 0) {
		return render::kMinCameraZoom;
	}
	const double fitX =
		static_cast<double>(screenWidth) / worldBounds.width;
	const double fitY =
		static_cast<double>(screenHeight) / worldBounds.height;
	const double fit = std::min(fitX, fitY) * render::kCameraFitRatio;
	return std::clamp(fit, render::kMinCameraZoom,
			  render::kMaxCameraZoom);
}

int CameraZoom::visibleSize(const int screenSize, const double zoom) const {
	if (zoom <= 0.0) {
		return screenSize;
	}
	return static_cast<int>(std::ceil(screenSize / zoom));
}

int CameraZoom::playVisibleSize(const int screenSize,
				const double zoom) const {
	const int playHeight = screenSize - render::kCameraHudBandPixels;
	if (playHeight <= 0) {
		return visibleSize(screenSize, zoom);
	}
	return visibleSize(playHeight, zoom);
}

int CameraZoom::axisOffset(const int targetCenter, const int worldOrigin,
			   const int worldSize, const int visibleSize,
			   const int playVisibleSize) const {
	// El mundo entra completo: se centra en el área de juego y la
	// cámara no scrollea. Si el mundo es más alto que esa área, se
	// pasa parejo por arriba y por abajo.
	if (worldSize <= visibleSize) {
		return worldOrigin -
		       (playVisibleSize - worldSize) /
			       render::kAxisCenterDivisor;
	}

	// El mundo no entra: sigue al target y se clava en los bordes.
	const int desired =
		targetCenter - visibleSize / render::kAxisCenterDivisor;
	const int maxOffset = worldSize - visibleSize;
	return std::clamp(desired, worldOrigin, worldOrigin + maxOffset);
}

}  // namespace sdl
