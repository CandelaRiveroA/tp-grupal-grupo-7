#pragma once

#include <string>

#include "toml/TomlDocument.h"
#include "world/DigitCatalog.h"

namespace world {

// Traduce digits.toml a un DigitCatalog (datos puros). Mismo patrón que
// Resource/Inventory TomlReader: clase propia, claves como constantes en
// el .cpp, sin funciones libres.
class DigitTomlReader {
public:
	explicit DigitTomlReader(const tomlcfg::TomlDocument& doc);

	DigitCatalog read() const;

private:
	std::string join(const std::string& base,
			 const std::string& key) const;

	const tomlcfg::TomlDocument& doc_;
};

}  // namespace world
