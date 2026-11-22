#pragma once

#include <string>

#include "world_model/Scenario.h"

namespace worldmodel {

class ScenarioSerializer {
public:
	Scenario loadFromFile(const std::string& path) const;
	Scenario loadFromString(const std::string& toml) const;
	void saveToFile(const Scenario& scenario,
			const std::string& path) const;
	std::string saveToString(const Scenario& scenario) const;
};

}
