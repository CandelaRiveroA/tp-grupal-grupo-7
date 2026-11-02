#include "world/CursorTomlReader.h"

#include <array>

#include "world/ConfigError.h"
#include "world/SpriteVisual.h"

namespace world {

namespace {

using tomlcfg::TomlDocument;

// Claves del TOML.
constexpr const char* kCursor = "cursor";
constexpr const char* kHotspot = "hotspot";
constexpr const char* kAt = "at";

// Variante del cursor -> tabla del TOML que la describe. Sumar una
// variante = una fila acá + su entrada en cursors.toml.
struct CursorBinding {
	CursorKind kind;
	const char* tableKey;
};

constexpr std::array<CursorBinding, 2> kCursorBindings = {{
	{CursorKind::Normal, "normal"},
	{CursorKind::Interactive, "interactive"},
}};

}

CursorTomlReader::CursorTomlReader(const TomlDocument& doc): doc_(doc) {}

CursorCatalog CursorTomlReader::read() const {
	CursorCatalog catalog;

	// Cada variante es un sprite completo (sheet/cell/at[/drawn]) que
	// lee el parser compartido; pueden venir de hojas distintas.
	for (const CursorBinding& binding : kCursorBindings) {
		catalog.visuals.emplace(binding.kind,
					SpriteVisual(doc_, binding.tableKey,
						     kAt));
	}

	// Hotspot: punto del sprite (en px) que se alinea con el puntero.
	const std::string hotspotPath =
		std::string(kCursor) + "." + kHotspot;
	const std::array<int, 2> hotspot = doc_.getIntArray<2>(hotspotPath);
	if (hotspot[0] < 0 || hotspot[1] < 0) {
		throw ConfigError(ConfigErrorKind::InvalidValue, hotspotPath);
	}
	catalog.hotspot = PixelPoint{hotspot[0], hotspot[1]};
	return catalog;
}

}
