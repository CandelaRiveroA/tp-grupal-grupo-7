#include "world_model/ScenarioTomlWriter.h"

#include <sstream>

#include <toml++/toml.hpp>

namespace worldmodel {

std::string ScenarioTomlWriter::write(const Scenario& scenario) const {
	std::ostringstream stream;
	stream << "format_version = 1\n\n[scenario]\n";

	toml::table scalars;
	scalars.insert("id", scenario.id);
	scalars.insert("kind", vocabulary_.toText(scenario.kind));
	scalars.insert("width", scenario.width);
	scalars.insert("height", scenario.height);
	scalars.insert("tile_size", scenario.tileSize);
	stream << scalars << '\n';

	fields_.appendBoolGrid(stream, "collision", scenario.collision,
			       scenario.width);
	fields_.appendTileGrid(stream, "ground", scenario.ground,
			       scenario.width);
	fields_.appendOptionalTileGrid(stream, "front", scenario.front,
				       scenario.width);

	toml::table tilesetsRoot;
	tilesetsRoot.insert("tilesets",
			    fields_.writeTilesets(scenario.tilesets));
	stream << '\n' << tilesetsRoot << '\n';
	return stream.str();
}

}
