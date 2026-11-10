#include "world_model/ScenarioLibrary.h"

#include <utility>

#include "world_model/FormatError.h"

namespace worldmodel {

void ScenarioLibrary::add(Scenario scenario) {
	if (scenario.id.empty() || find(scenario.id) != nullptr) {
		throw FormatError(FormatErrorKind::InvalidValue, "scenario");
	}
	scenarios_.push_back(std::move(scenario));
}

const Scenario* ScenarioLibrary::find(const std::string& id) const {
	for (const Scenario& scenario : scenarios_) {
		if (scenario.id == id) {
			return &scenario;
		}
	}
	return nullptr;
}

const std::vector<Scenario>& ScenarioLibrary::all() const {
	return scenarios_;
}

}
