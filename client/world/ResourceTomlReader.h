#pragma once

#include <map>
#include <string>

#include "toml/TomlDocument.h"
#include "world/ResourceCatalog.h"

namespace world {

// Traduce resources.toml a un ResourceCatalog (datos puros).
// Mismo patrón que Inventory/ToolOverlay/CharacterTomlReader: clase
// propia, métodos de instancia, claves como constantes (en el .cpp), sin
// funciones libres ni helpers static. Las secciones [resources.*] se
// recorren por su contentId (vocabulario abierto, como [crops.*]).
class ResourceTomlReader {
public:
	explicit ResourceTomlReader(const tomlcfg::TomlDocument& doc);

	ResourceCatalog read() const;

private:
	std::string join(const std::string& base,
			 const std::string& key) const;

	std::map<std::string, ResourceState> stateNames() const;

	ResourceVisual readVisual(const std::string& base,
				  const std::string& sheet, int cellWidth,
				  int cellHeight, const std::string& statePath) const;

	void readResource(const std::string& contentId,
			  const std::string& base,
			  ResourceCatalog& catalog) const;

	const tomlcfg::TomlDocument& doc_;
};

}
