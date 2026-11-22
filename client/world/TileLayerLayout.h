#pragma once

#include "world/Grid.h"
#include "world/ScenePixelMapper.h"

namespace world {

// Geometría pura de una capa de tiles en grilla: qué celdas caen dentro
// de lo visible y dónde (px de mundo) está cada una. Sin SDL: el pintor
// solo recorre el rango y dibuja. Usa el ScenePixelMapper (única
// conversión celda <-> px del proyecto).
class TileLayerLayout {
public:
	struct Range {
		int firstColumn;
		int lastColumn;
		int firstRow;
		int lastRow;

		bool empty() const;
	};

	explicit TileLayerLayout(const ScenePixelMapper& mapper);

	// Celdas de una capa de `size` que tocan el rectángulo visible (px
	// de mundo), recortadas a la capa. Vacío si no se ve ninguna.
	Range visible(const GridSpan& size, const PixelRect& visibleWorld) const;

	// Caja de una celda en px de mundo.
	PixelRect cellRect(int column, int row) const;

private:
	ScenePixelMapper mapper_;
};

}
