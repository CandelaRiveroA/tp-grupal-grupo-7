#pragma once

#include <map>
#include <optional>
#include <string>

#include "world_model/Portal.h"

namespace worldmodel {
class PrefabCatalog;
struct PrefabDef;
}

// Vinculo 1:1 entre portales de mazmorra y su prefab visual en prefabs.toml.
class PortalPrefabs {
public:
	PortalPrefabs();

	std::optional<worldmodel::PortalKind> kindForPrefabId(
		const std::string& prefabId) const;
	const worldmodel::PrefabDef* prefabFor(
		worldmodel::PortalKind kind,
		const worldmodel::PrefabCatalog& catalog) const;

private:
	std::map<worldmodel::PortalKind, std::string> kindToId_;
	std::map<std::string, worldmodel::PortalKind> idToKind_;
};
