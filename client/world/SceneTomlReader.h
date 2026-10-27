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

	void readElements(const std::string& roomBase, int defaultScale,
			  SceneDescription& scene) const;

	DrawSpec readDrawSpec(const tomlcfg::TomlDocument& draw,
			      int defaultScale,
			      const std::string& context) const;

	DrawMode parseDrawMode(const std::string& mode,
			       const std::string& context) const;

	std::string join(const std::string& base,
			 const std::string& key) const;

	const tomlcfg::TomlDocument& doc_;
};

}
