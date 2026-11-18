#include "world_model/ScenarioSerializer.h"

#include "toml/TomlDocument.h"
#include "world_model/ScenarioTomlReader.h"
#include "world_model/ScenarioTomlWriter.h"
#include "world_model/SerializerIO.h"

namespace worldmodel {

Scenario ScenarioSerializer::loadFromFile(const std::string& path) const {
	return SerializerIO::load<Scenario, ScenarioTomlReader>(
		path, tomlcfg::TomlDocument::Source::File);
}

Scenario ScenarioSerializer::loadFromString(const std::string& toml) const {
	return SerializerIO::load<Scenario, ScenarioTomlReader>(
		toml, tomlcfg::TomlDocument::Source::Text);
}

void ScenarioSerializer::saveToFile(const Scenario& scenario,
				    const std::string& path) const {
	SerializerIO::save(saveToString(scenario), path);
}

std::string ScenarioSerializer::saveToString(
	const Scenario& scenario) const {
	return ScenarioTomlWriter().write(scenario);
}

}
