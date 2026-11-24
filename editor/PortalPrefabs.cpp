#include "PortalPrefabs.h"

#include "world_model/PrefabCatalog.h"

PortalPrefabs::PortalPrefabs()
	: kindToId_{
		  {worldmodel::PortalKind::StairsDown, "escalera_bajada"},
		  {worldmodel::PortalKind::DungeonExit, "salida_mazmorra"},
	  } {
	for (const auto& [kind, id] : kindToId_) {
		idToKind_.emplace(id, kind);
	}
}

std::optional<worldmodel::PortalKind> PortalPrefabs::kindForPrefabId(
	const std::string& prefabId) const {
	const auto it = idToKind_.find(prefabId);
	if (it == idToKind_.end()) {
		return std::nullopt;
	}
	return it->second;
}

const worldmodel::PrefabDef* PortalPrefabs::prefabFor(
	worldmodel::PortalKind kind,
	const worldmodel::PrefabCatalog& catalog) const {
	const auto it = kindToId_.find(kind);
	if (it == kindToId_.end()) {
		return nullptr;
	}
	return catalog.find(it->second);
}
