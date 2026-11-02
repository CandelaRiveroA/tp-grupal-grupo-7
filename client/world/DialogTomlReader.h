#pragma once

#include <string>

#include "toml/TomlDocument.h"
#include "world/DialogCatalog.h"

namespace world {

// Traduce ui/dialogs.toml a un DialogCatalog (datos puros). Mismo patrón
// que CursorTomlReader: clase propia, claves como constantes en el .cpp,
// sin funciones libres. Las hojas/celdas se leen con el parser compartido
// (SpriteVisual / SheetCell).
class DialogTomlReader {
public:
	explicit DialogTomlReader(const tomlcfg::TomlDocument& doc);

	DialogCatalog read() const;

private:
	std::string join(const std::string& base, const std::string& key) const;
	std::string idx(const std::string& base, std::size_t i) const;

	DialogAnimation readAnimation(const std::string& base) const;
	DialogData readDialog(const std::string& base) const;
	PixelPoint readOffset(const std::string& key) const;

	const tomlcfg::TomlDocument& doc_;
};

}  // namespace world
