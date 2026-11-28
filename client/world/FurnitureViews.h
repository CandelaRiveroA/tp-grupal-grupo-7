#pragma once

#include <map>
#include <string>
#include <vector>

#include "world/ContentCatalog.h"
#include "world/FurnitureFacing.h"
#include "world/Grid.h"

namespace world {

// Cómo se ve un mueble en una orientación: su sprite (nullptr si no
// tiene) y si se dibuja espejado.
struct FurnitureView {
	const SpriteVisual* visual = nullptr;
	bool mirrored = false;
};

// Las orientaciones de un mueble a partir de sus vistas en
// inventory.toml (`<id>`, `<id>_side`, `<id>_back`). Una tabla miembro
// dice, por orientación, qué vista usa, si va espejada y si intercambia
// la huella: sumar una orientación es una fila más. Un mueble solo puede
// girar a las orientaciones cuya vista existe (una mesa no gira; un
// sillón sin `back` pasa de costado a costado). Pura: sin SDL.
class FurnitureViews {
public:
	explicit FurnitureViews(const ContentCatalog& content);

	// La siguiente orientación disponible del ciclo (la misma si no
	// tiene otra).
	FurnitureFacing next(const std::string& contentId,
			     FurnitureFacing current) const;

	// El sprite de esa orientación; sin esa vista, el de frente.
	FurnitureView viewOf(const std::string& contentId,
			     FurnitureFacing facing) const;

	// La huella de piso en esa orientación (de costado se intercambian
	// columnas y filas).
	GridSpan footprintOf(const GridSpan& span, FurnitureFacing facing) const;

private:
	struct Row {
		std::string suffix;
		bool mirrored;
		bool swapsFootprint;
	};

	bool has(const std::string& contentId, FurnitureFacing facing) const;

	const ContentCatalog& content_;
	std::map<FurnitureFacing, Row> rows_;
	std::vector<FurnitureFacing> cycle_;
};

}
