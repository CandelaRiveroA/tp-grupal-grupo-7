#pragma once

#include <map>
#include <string>

namespace world {

// Datos puros de las fuentes de texto (sin SDL): el tamaño por defecto y
// el archivo de cada CARA con nombre. La cara "default" es la de [font]
// file; las demás salen de las entradas [[font.face]] (name + file) de
// ui/text.toml. La traducción vive en su constructor (§2): carga, valida y
// lanza ConfigError. El que dibuja (sdl::TextRenderer) resuelve el archivo.
struct FontCatalog {
	std::string file;
	int size = 0;
	// nombre de cara -> archivo (relativo a assets/). Incluye "default".
	std::map<std::string, std::string> faces;

	explicit FontCatalog(const std::string& path);

	// Archivo de la cara pedida; ConfigError(InvalidValue) si no existe.
	const std::string& fileFor(const std::string& face) const;
};

}  // namespace world
