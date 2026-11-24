#pragma once

#include <string>

#include "world_model/FieldWriter.h"
#include "world_model/Scenario.h"
#include "world_model/WorldVocabulary.h"

namespace worldmodel {

class ScenarioTomlWriter {
public:
	std::string write(const Scenario& scenario) const;

private:
	FieldWriter fields_;
	WorldVocabulary vocabulary_;
};

}
