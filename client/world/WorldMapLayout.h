#pragma once

#include "world/PagedModal.h"
#include "world/PixelBox.h"
#include "world/WorldMapConfig.h"

namespace world {

// Geometría pura del overlay del mapa del mundo. Reutiliza PagedModalLayout
// para el panel, pergamino, flechas y hit-testing.
class WorldMapLayout {
public:
	WorldMapLayout(int screenW, int screenH, const WorldMapConfig& config,
		       const PixelSize& mapTextureSize);

	const PagedModalLayout& modal() const;
	const PixelBox& titleBox() const;
	const PixelBox& mapRect() const;
	PixelBox mapBorderBox() const;
	PixelBox pinOuterBox(float normX, float normY) const;
	PixelBox pinInnerBox(const PixelBox& outer) const;

	bool hitsSwitchArrow(int x, int y) const;

private:
	PagedModalLayout modal_;
	PixelBox titleBox_{};
	PixelBox mapRect_{};
	int mapBorder_ = 0;
	int pinSize_ = 0;
	int pinBorder_ = 0;
};

}  // namespace world
