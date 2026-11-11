#pragma once

#include <string>

#include "toml/TomlDocument.h"
#include "world/BuildPlaceCatalog.h"

namespace world {

// Traduce buildplace.toml a un BuildPlaceCatalog (datos puros).
// Mismo patrón que GroundTomlReader / ResourceTomlReader.
class BuildPlaceTomlReader {
public:
	explicit BuildPlaceTomlReader(const tomlcfg::TomlDocument& doc);

	BuildPlaceCatalog read() const;

private:
	std::string join(const std::string& base, const std::string& key) const;

	// Nombre de tabla del TOML -> estado (tabla, no switch §4).
	// nullptr si el nombre no es un estado conocido.
	const BuildPlaceState* stateFor(const std::string& name) const;

	BuildPlaceVisual readVisual(const std::string& sheet, int cellWidth,
				    int cellHeight,
				    const std::string& base) const;

	const tomlcfg::TomlDocument& doc_;
};

}
