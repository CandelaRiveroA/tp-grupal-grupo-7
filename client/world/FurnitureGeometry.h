#pragma once

#include <optional>
#include <string>

#include "world/ContentCatalog.h"
#include "world/FurnitureFacing.h"
#include "world/FurnitureViews.h"
#include "world/Grid.h"
#include "world/NpcCatalog.h"
#include "world/ScenePixelMapper.h"

namespace world {

// Dónde y cuánto ocupa un mueble en la grilla. DTO plano.
//   base:    celda abajo-izquierda del sprite.
//   topLeft: celda arriba-izquierda (de donde parte la caja).
//   cells:   celdas lógicas que abarca el sprite (no la huella de piso).
//   box:     caja en px de mundo (lo que se dibuja y se toca).
//   visual/mirrored: el sprite de su orientación (nullptr en los
//            muebles por piezas, que dibuja su definición de npcs.toml).
struct FurnitureShape {
	GridCell base;
	GridCell topLeft;
	GridSpan cells;
	PixelRect box;
	const SpriteVisual* visual = nullptr;
	bool mirrored = false;
};

// Geometría ÚNICA de un mueble a partir de su contentId y la celda que
// manda la fuente: la usan el dibujo, el click, la colocación y la cama,
// así lo que se toca es lo que se ve. Un mueble es un bloque de
// inventory.toml (FurnitureLayout) o, si npcs.toml tiene un objeto con
// ese id, un mueble POR PIEZAS (animado como el cofre o no contiguo
// como la cama), con su ancla. Pura: no conoce SDL.
class FurnitureGeometry {
public:
	FurnitureGeometry(const ContentCatalog& content, const NpcCatalog& props);

	// La definición por piezas del mueble (nullptr si es un bloque).
	const NpcDef* prop(const std::string& contentId) const;

	// Celda base (abajo-izquierda) a partir de la celda de la fuente,
	// y al revés (lo que se manda al colocar con base en `base`).
	GridCell baseOf(const std::string& contentId, const GridCell& source) const;
	GridCell sourceOf(const std::string& contentId, const GridCell& base) const;

	// Las orientaciones de los muebles (qué vista usa cada una).
	const FurnitureViews& views() const;

	// Forma del mueble con su base en `base` y mirando a `facing`
	// (nullopt si no tiene visual). Los muebles por piezas no giran.
	std::optional<FurnitureShape> shapeAtBase(
		const std::string& contentId, const GridCell& base,
		const ScenePixelMapper& mapper,
		FurnitureFacing facing = FurnitureFacing::Front) const;

	// Igual, desde la celda que manda la fuente.
	std::optional<FurnitureShape> shapeOf(
		const std::string& contentId, const GridCell& source,
		const ScenePixelMapper& mapper,
		FurnitureFacing facing = FurnitureFacing::Front) const;

private:
	GridCell anchorOf(const std::string& contentId) const;

	// Celdas que hacen falta para cubrir `pixels` (redondeo hacia arriba).
	int cellsFor(int pixels, int cellPixelSize) const;

	const ContentCatalog& content_;
	const NpcCatalog& props_;
	FurnitureViews views_;
};

}
