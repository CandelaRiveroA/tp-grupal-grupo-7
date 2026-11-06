#pragma once

#include <cstdint>
#include <string>

#include "toml/TomlDocument.h"
#include "world/GroundCatalog.h"

namespace world {

// Traduce ground.toml a un GroundCatalog (datos puros).
// Mismo patrón que CropTomlReader / ResourceTomlReader: clase propia,
// métodos de instancia, claves como constantes (en el .cpp), sin
// funciones libres ni helpers static.
class GroundTomlReader {
public:
	explicit GroundTomlReader(const tomlcfg::TomlDocument& doc);

	GroundCatalog read() const;

private:
	std::string join(const std::string& base, const std::string& key) const;

	std::string idx(const std::string& base, std::size_t i) const;

	// Nombre de tabla del TOML -> consecuencia (tabla, no switch §4).
	// nullptr si el nombre no es una consecuencia conocida.
	const GroundConsequence* consequenceFor(const std::string& name) const;

	// Nombre de pieza del TOML -> máscara de vecinos (tabla, no switch
	// §4). nullptr si el nombre no es una pieza conocida.
	const std::uint8_t* maskFor(const std::string& role) const;

	SpriteVisual readSprite(const std::string& sheet, int cellWidth,
				int cellHeight,
				const std::string& cellPath) const;

	// Lee `[ground.<n>.tiles]`: pieza -> variantes de sprite. Si la
	// tabla no está, la consecuencia se dibuja con su sprite simple.
	void readPieces(const std::string& tilesPath, const std::string& sheet,
			int cellWidth, int cellHeight,
			GroundVisual& visual) const;

	const tomlcfg::TomlDocument& doc_;
};

}
