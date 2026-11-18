#pragma once

#include <string>

#include "world_model/PrefabCatalog.h"

namespace worldmodel {

class PrefabCatalogLoader {
public:
	PrefabCatalog loadFromFile(const std::string& path) const;
};

}
