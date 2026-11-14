#pragma once

#include <string>

#include <toml++/toml.hpp>

#include "world_model/Scenario.h"
#include "world_model/WorldVocabulary.h"

namespace worldmodel {

class ScenarioTomlWriter {
public:
	std::string write(const Scenario& scenario) const;

private:
	toml::table writeScenario(const Scenario& scenario) const;
	WorldVocabulary vocabulary_;
};

}
