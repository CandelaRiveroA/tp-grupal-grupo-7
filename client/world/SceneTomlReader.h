#pragma once

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

	std::string join(const std::string& base,
			 const std::string& key) const;

	std::string idx(const std::string& base, std::size_t i) const;

	const tomlcfg::TomlDocument& doc_;
};

}
