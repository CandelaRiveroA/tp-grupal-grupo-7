#pragma once

#include <string>

#include "world_model/ScenarioLibrary.h"

namespace worldmodel {

class ScenarioLibraryLoader {
public:
	ScenarioLibrary loadFromDirectory(const std::string& path) const;
};

}
