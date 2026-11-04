#pragma once

#include <map>

#include "world/SceneDescription.h"
#include "world/ScenePixelMapper.h"

namespace world {

// Geometría pura de un elemento de escena. No conoce SDL ni assets: solo
// calcula el tamaño de pieza y el rectángulo que realmente ocupa el
// elemento, para que pintado e índice espacial compartan el mismo cálculo.
class ElementLayout {
public:
	ElementLayout(const TileSetDef& tileset,
		      const ElementPlacement& element,
		      const PixelRect& area);

	const PixelRect& area() const;
	const ElementPlacement& element() const;
	DrawMode mode() const;
	int pieceWidth() const;
	int pieceHeight() const;
	PixelRect bounds() const;

private:
	const TileSetDef& tileset_;
	const ElementPlacement& element_;
	const PixelRect area_;
	std::map<DrawMode, PixelRect> boundsByMode_;
};

}  // namespace world
