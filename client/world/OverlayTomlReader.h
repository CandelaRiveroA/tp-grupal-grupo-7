#pragma once

#include "toml/TomlDocument.h"
#include "world/WorldOverlay.h"

namespace world {

// Traduce overlay.toml a WorldOverlay (datos puros) y valida lo que
// puede sin conocer el mundo; lo que depende del mundo (hojas y tiles
// que existan) lo valida quien lo aplica. Todo el archivo es opcional.
class OverlayTomlReader {
public:
	explicit OverlayTomlReader(const tomlcfg::TomlDocument& doc);

	WorldOverlay read() const;

private:
	// `[world] scale` (entero > 0); sin la key vale el default.
	void readScale(WorldOverlay& overlay) const;

	// `[decorations."tileset:col,row"]` -> variantes de noche.
	void readDecorations(WorldOverlay& overlay) const;

	// `[[maps.<id>.elements]]` -> decoraciones iniciales, con la misma
	// sintaxis (y el mismo lector) que scenes.toml.
	void readMapElements(WorldOverlay& overlay) const;

	const tomlcfg::TomlDocument& doc_;
};

}
