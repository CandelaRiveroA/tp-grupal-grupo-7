#pragma once

#include <array>
#include <string>

#include "toml/TomlDocument.h"
#include "world/CropCatalog.h"

namespace world {

// Traduce crops.toml a un CropCatalog (datos puros).
// Mismo patrón que ResourceTomlReader / InventoryTomlReader: clase
// propia, métodos de instancia, claves como constantes (en el .cpp), sin
// funciones libres ni helpers static.
class CropTomlReader {
public:
	explicit CropTomlReader(const tomlcfg::TomlDocument& doc);

	CropCatalog read() const;

private:
	std::string join(const std::string& base,
			 const std::string& key) const;

	std::string idx(const std::string& base, std::size_t i) const;

	// Orden de las calidades en el array del TOML (base -> perfecta).
	// Tabla, no switch (convención §4).
	std::array<CropQuality, 5> qualityOrder() const;

	SpriteVisual readSprite(const std::string& sheet, int cellWidth,
				int cellHeight,
				const std::string& cellPath) const;

	std::vector<SpriteVisual> readStages(const std::string& sheet,
					     int cellWidth, int cellHeight,
					     const std::string& path) const;

	CropDefinition readCrop(const std::string& base, const std::string& sheet,
				int cellWidth, int cellHeight) const;

	const tomlcfg::TomlDocument& doc_;
};

}
