#pragma once

#include <string>
#include <vector>

#include "world_model/Scenario.h"

namespace worldmodel {

class ScenarioLibrary {
public:
	void add(Scenario scenario);
	const Scenario* find(const std::string& id) const;
	std::vector<const Scenario*> byKind(MapKind kind) const;
	const std::vector<Scenario>& all() const;

private:
	std::vector<Scenario> scenarios_;
};

}
