#pragma once

#include <string>

namespace world {

// Datos puros de una fuente de texto (sin SDL): qué archivo de fuente y
// qué tamaño por defecto. La traducción vive en su constructor (§2):
// carga la sección [font] de ui/text.toml, valida y lanza ConfigError.
// El que dibuja (sdl::TextRenderer) resuelve el archivo.
struct FontCatalog {
	std::string file;
	int size = 0;

	explicit FontCatalog(const std::string& path);
};

}  // namespace world
