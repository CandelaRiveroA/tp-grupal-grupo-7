#pragma once

#include "toml/TomlDocument.h"
#include "world_model/Scenario.h"
#include "world_model/FieldReader.h"
#include "world_model/WorldVocabulary.h"

namespace worldmodel {

class ScenarioTomlReader {
public:
	explicit ScenarioTomlReader(const tomlcfg::TomlDocument& doc);

	Scenario read() const;

private:
	const tomlcfg::TomlDocument& doc_;
	FieldReader fields_;
	WorldVocabulary vocabulary_;
};

}
