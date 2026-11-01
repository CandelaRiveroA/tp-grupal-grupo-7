#pragma once

#include <array>
#include <string>

#include "toml/TomlDocument.h"
#include "world/CursorCatalog.h"

namespace world {

// Traduce ui/cursors.toml a un CursorCatalog (datos puros). Mismo patrón
// que DigitTomlReader: clase propia, claves como constantes en el .cpp,
// sin funciones libres. Los sprites de cada variante se leen con el
// parser compartido (SpriteVisual).
class CursorTomlReader {
public:
	explicit CursorTomlReader(const tomlcfg::TomlDocument& doc);

	CursorCatalog read() const;

private:
	const tomlcfg::TomlDocument& doc_;
};

}  // namespace world
