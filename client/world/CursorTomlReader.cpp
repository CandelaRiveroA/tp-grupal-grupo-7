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
	// Las variantes nuevas son opcionales: un TOML viejo sigue cargando
	// (y el painter cae a otra variante). Las históricas son obligatorias.
	bool required;
};

constexpr std::array<CursorBinding, 4> kCursorBindings = {{
	{CursorKind::Normal, "normal", true},
	{CursorKind::Interactive, "interactive", true},
	{CursorKind::Gift, "gift", false},
	{CursorKind::Grab, "grab", false},
}};

}

CursorTomlReader::CursorTomlReader(const TomlDocument& doc): doc_(doc) {}

CursorCatalog CursorTomlReader::read() const {
	CursorCatalog catalog;

	// Cada variante es un sprite completo (sheet/cell/at[/drawn]) que
	// lee el parser compartido; pueden venir de hojas distintas.
	for (const CursorBinding& binding : kCursorBindings) {
		if (!binding.required && !doc_.hasKey(binding.tableKey)) {
			continue;
		}
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
