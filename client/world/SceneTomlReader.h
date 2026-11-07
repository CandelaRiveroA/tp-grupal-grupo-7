#pragma once

#include <map>
#include <string>

#include "toml/TomlDocument.h"
#include "world/SceneDescription.h"

namespace world {

class SceneTomlReader {
public:
	explicit SceneTomlReader(const tomlcfg::TomlDocument& doc);

	SceneDescription readRoom(const std::string& roomName) const;

private:
	void readTileSets(SceneDescription& scene) const;

	// Poblaciones OPCIONALES de la sala ([rooms.X.spawns]): qué puebla
	// la fuente de estado. Sin la subtabla, la sala no puebla nada.
	void readSpawns(const std::string& roomBase,
			SceneDescription& scene) const;

	void readElements(const std::string& roomBase, int defaultScale,
			  SceneDescription& scene) const;

	// Lee la pieza del elemento y la normaliza a tamaño + una celda de
	// hoja por posición (dos formas de TOML, un solo formato interno).
	void readPiece(const tomlcfg::TomlDocument& element,
		       const std::string& context,
		       ElementPlacement& placement) const;

	DrawSpec readDrawSpec(const tomlcfg::TomlDocument& draw,
			      int defaultScale,
			      const std::string& context) const;

	DrawMode parseDrawMode(const std::string& mode,
			       const std::string& context) const;

	// "Entrar" al elemento (OPCIONAL, subtabla `enter`): traduce y
	// valida; sin la subtabla no hace nada.
	void readEnter(const tomlcfg::TomlDocument& element,
		       const std::string& context,
		       ElementPlacement& placement) const;

	// Momento del día del elemento (OPCIONAL, key `when`): traduce y
	// valida; sin la key el elemento se dibuja en cualquier momento.
	void readWhen(const tomlcfg::TomlDocument& element,
		      const std::string& context,
		      ElementPlacement& placement) const;

	// Nombre de dirección del TOML -> vocabulario. Lanza
	// ConfigError(InvalidValue, path) si el nombre no existe.
	Direction parseFacing(const std::string& name,
			      const std::string& context) const;

	// Nombre de momento del día del TOML -> vocabulario. Lanza
	// ConfigError(InvalidValue, path) si el nombre no existe.
	TimeOfDay parseTimeOfDay(const std::string& name,
				 const std::string& context) const;

	std::string join(const std::string& base,
			 const std::string& key) const;

	std::string idx(const std::string& base, std::size_t i) const;

	// Nombres de dirección del TOML -> Direction (diccionario, no
	// switch): se consulta con find, nunca con .at() sobre dato crudo.
	std::map<std::string, Direction> directionsByName_;

	// Nombres de momento del día del TOML -> TimeOfDay (mismo
	// patrón de diccionario).
	std::map<std::string, TimeOfDay> timeOfDayByName_;

	const tomlcfg::TomlDocument& doc_;
};

}
