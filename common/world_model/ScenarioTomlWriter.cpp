#include "world_model/ScenarioTomlWriter.h"

#include <sstream>

#include <toml++/toml.hpp>

#include "world_model/FieldWriter.h"

namespace worldmodel {

std::string ScenarioTomlWriter::write(const Scenario& scenario) const {
	toml::table table;
	table.insert("format_version", 1);
	table.insert("scenario", writeScenario(scenario));
	table.insert("tilesets",
		     FieldWriter::writeTilesets(scenario.tilesets));

	std::ostringstream stream;
	stream << table;
	return stream.str();
}

toml::table ScenarioTomlWriter::writeScenario(
	const Scenario& scenario) const {
	toml::table table;
	table.insert("id", scenario.id);
	table.insert("kind", vocabulary_.toText(scenario.kind));
	table.insert("width", scenario.width);
	table.insert("height", scenario.height);
	table.insert("tile_size", scenario.tileSize);
	table.insert("collision",
		     FieldWriter::writeBooleans(scenario.collision));
	table.insert("ground", FieldWriter::writeGrid(scenario.ground));
	return table;
}

}
