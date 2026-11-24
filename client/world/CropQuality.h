#pragma once

namespace world {

// Calidad del producto cosechado (vocabulario del juego, lo comunica el
// servidor; alineado con Quality de server/domain/inventory/item_types.h).
// El orden del enum es el orden de menor a mayor calidad.
enum class CropQuality {
	Base,
	Good,
	Excellent,
	Incredible,
	Perfect,
};

}
