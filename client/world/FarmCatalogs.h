#pragma once

#include <string>

#include "world/BuildPlaceCatalog.h"
#include "world/CropCatalog.h"
#include "world/GroundCatalog.h"
#include "world/InventoryLayout.h"

namespace world {

// Los catálogos del campo (cultivos, terreno y lugar de plantado), todos
// datos puros leídos de TOML. Es lo único que sabe armarlos y lo que
// suma los contenidos de los cultivos (sacos y cosecha) al inventario,
// para que el HUD sepa dibujar lo que el servidor mande por su id.
class FarmCatalogs {
public:
	FarmCatalogs(const std::string& cropsPath,
		     const std::string& groundPath,
		     const std::string& buildPlacePath);

	FarmCatalogs(CropCatalog crops, GroundCatalog ground,
		     BuildPlaceCatalog buildPlaces);

	// Copia del inventario con los sacos y la cosecha registrados
	// (apariencia y rasgos). No modifica el original.
	InventoryLayout withCropContents(const InventoryLayout& base) const;

	const CropCatalog& crops() const;
	const GroundCatalog& ground() const;
	const BuildPlaceCatalog& buildPlaces() const;

private:
	CropCatalog crops_;
	GroundCatalog ground_;
	BuildPlaceCatalog buildPlaces_;
};

}
