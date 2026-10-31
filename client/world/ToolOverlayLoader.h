#pragma once

#include <string>

#include "world/ToolOverlay.h"

namespace world {

// Carga tools_overlay.toml hacia un ToolOverlayCatalog.
// Mismo patrón que CharacterCatalogLoader / InventoryConfigLoader:
// TomlDocument -> reader interno -> structs de datos puros.
// Lanza ConfigError si el archivo es inválido.
class ToolOverlayLoader {
public:
	ToolOverlayCatalog loadFromFile(const std::string& path) const;

	ToolOverlayCatalog loadFromString(const std::string& toml) const;
};

}
