#include "world_model/MapRules.h"

#include <algorithm>
#include <map>
#include <string>
#include <vector>

namespace worldmodel {

MapRules::MapRules()
	: markerSpecs_{
		  {MarkerKind::Bed, MarkerSpec{2, true, false}},
		  {MarkerKind::GeneralWindow, MarkerSpec{2, false, true}},
		  {MarkerKind::CoopWindow, MarkerSpec{1, false, true}},
	  } {
	const auto add = [this](const std::string& id, MapKind kind) -> MapRule& {
		rules_.push_back(MapRule{});
		MapRule& rule = rules_.back();
		rule.id = id;
		rule.kind = kind;
		return rule;
	};

	MapRule& farm = add("farm", MapKind::Farm);
	farm.requiredObjects = {ObjectRole::Barn};
	farm.requiredPortals = {PortalKind::TownEntrance,
				PortalKind::DungeonEntrance};

	MapRule& town = add("town", MapKind::Town);
	town.requiredObjects = {ObjectRole::Carpentry, ObjectRole::Blacksmith,
				ObjectRole::GeneralStore, ObjectRole::Ranch,
				ObjectRole::CommunityCenter};
	town.requiredPortals = {PortalKind::FarmEntrance};

	for (int k = kMinPlayers; k <= kMaxPlayers; ++k) {
		MapRule& cabin = add(cabinId(k), MapKind::Cabin);
		cabin.allowedMarkers = {MarkerKind::Bed,
					MarkerKind::GeneralWindow,
					MarkerKind::CoopWindow};
		cabin.requiredMarkers = {{MarkerKind::Bed, true}};
		cabin.requiredPortals = {PortalKind::InteriorExit};
		cabin.interiorObject = ObjectRole::Cabin;
		cabin.interiorExitTarget = "farm";
	}

	struct ShopRuleSpec {
		const char* id;
		MapKind mapKind;
		ObjectRole role;
		MarkerKind merchant;
	};
	const ShopRuleSpec shops[] = {
		{"carpentry", MapKind::Carpentry, ObjectRole::Carpentry,
		 MarkerKind::MerchantRobin},
		{"blacksmith", MapKind::Blacksmith, ObjectRole::Blacksmith,
		 MarkerKind::MerchantClint},
		{"general_store", MapKind::GeneralStore,
		 ObjectRole::GeneralStore, MarkerKind::MerchantPierre},
		{"ranch", MapKind::Ranch, ObjectRole::Ranch,
		 MarkerKind::MerchantMarnie},
	};
	for (const ShopRuleSpec& spec : shops) {
		MapRule& shop = add(spec.id, spec.mapKind);
		shop.allowedMarkers = {spec.merchant, MarkerKind::GeneralWindow,
				       MarkerKind::CoopWindow};
		shop.requiredMarkers = {{spec.merchant, true}};
		shop.requiredPortals = {PortalKind::InteriorExit};
		shop.interiorObject = spec.role;
		shop.interiorExitTarget = "town";
	}

	MapRule& barn = add("barn", MapKind::Barn);
	barn.allowedMarkers = {MarkerKind::GeneralWindow,
			       MarkerKind::CoopWindow};
	barn.requiredPortals = {PortalKind::InteriorExit};
	barn.interiorObject = ObjectRole::Barn;
	barn.interiorExitTarget = "farm";

	for (int level = 1; level <= kDungeonLevels; ++level) {
		MapRule& dungeon = add(dungeonId(level), MapKind::Dungeon);
		dungeon.requiredPortals = {PortalKind::DungeonExit};
		if (level < kDungeonLevels) {
			dungeon.requiredPortals.push_back(
				PortalKind::StairsDown);
		}
	}
}

const std::vector<MapRule>& MapRules::all() const { return rules_; }

const MapRule* MapRules::find(const std::string& id) const {
	for (const MapRule& rule : rules_) {
		if (rule.id == id) {
			return &rule;
		}
	}
	return nullptr;
}

std::vector<std::string> MapRules::requiredMapIds(int cabinCount) const {
	std::vector<std::string> ids;
	ids.reserve(rules_.size());
	for (const MapRule& rule : rules_) {
		const std::optional<int> cabin = cabinIndex(rule.id);
		if (cabin.has_value() && *cabin > cabinCount) {
			continue;
		}
		ids.push_back(rule.id);
	}
	return ids;
}

std::optional<std::string> MapRules::requiredHostMap(ObjectRole role) const {
	for (const MapRule& rule : rules_) {
		if (std::find(rule.requiredObjects.begin(),
			      rule.requiredObjects.end(),
			      role) != rule.requiredObjects.end()) {
			return rule.id;
		}
	}
	for (const MapRule& rule : rules_) {
		if (rule.interiorObject == role &&
		    rule.interiorExitTarget.has_value()) {
			return rule.interiorExitTarget;
		}
	}
	return std::nullopt;
}

std::vector<std::string> MapRules::fixedInteriorIds() const {
	std::vector<std::string> ids;
	for (const MapRule& rule : rules_) {
		if (rule.interiorObject.has_value() &&
		    rule.kind != MapKind::Cabin) {
			ids.push_back(rule.id);
		}
	}
	return ids;
}

bool MapRules::isInteriorHost(const std::string& mapId) const {
	return std::any_of(rules_.begin(), rules_.end(),
			   [&](const MapRule& rule) {
				   return rule.interiorExitTarget == mapId;
			   });
}

bool MapRules::isValidMapId(const std::string& id) const {
	if (id.empty()) {
		return false;
	}
	for (const char character : id) {
		const bool valid = (character >= 'a' && character <= 'z') ||
				   (character >= '0' && character <= '9') ||
				   character == '-' || character == '_';
		if (!valid) {
			return false;
		}
	}
	return true;
}

// El nombre es tambien el nombre de archivo (<nombre>.toml), asi que se
// rechaza lo que romperia el path.
bool MapRules::isValidWorldName(const std::string& name) const {
	return !name.empty() && name != "." && name != ".." &&
	       name.find('/') == std::string::npos &&
	       name.find('\0') == std::string::npos;
}

std::string MapRules::cabinId(int k) const {
	return "cabin_" + std::to_string(k);
}

std::optional<int> MapRules::cabinIndex(const std::string& id) const {
	for (int k = kMinPlayers; k <= kMaxPlayers; ++k) {
		if (id == cabinId(k)) {
			return k;
		}
	}
	return std::nullopt;
}

std::string MapRules::dungeonId(int level) const {
	return "dungeon_" + std::to_string(level);
}

std::optional<int> MapRules::dungeonLevel(const std::string& id) const {
	for (int level = 1; level <= kDungeonLevels; ++level) {
		if (id == dungeonId(level)) {
			return level;
		}
	}
	return std::nullopt;
}

std::optional<std::string> MapRules::expectedPortalTarget(
	const std::string& mapId, PortalKind kind) const {
	static const std::map<PortalKind, std::string> FixedTargets = {
		{PortalKind::TownEntrance, "town"},
		{PortalKind::FarmEntrance, "farm"},
		{PortalKind::DungeonEntrance, "dungeon_1"},
		{PortalKind::DungeonExit, "farm"},
	};
	if (const auto it = FixedTargets.find(kind);
	    it != FixedTargets.end()) {
		return it->second;
	}
	if (kind == PortalKind::InteriorExit) {
		const MapRule* rule = find(mapId);
		return rule != nullptr ? rule->interiorExitTarget
				       : std::nullopt;
	}
	if (kind == PortalKind::StairsDown) {
		const std::optional<int> level = dungeonLevel(mapId);
		if (level.has_value() && *level < kDungeonLevels) {
			return dungeonId(*level + 1);
		}
	}
	return std::nullopt;
}

MarkerSpec MapRules::markerSpec(MarkerKind kind) const {
	const auto it = markerSpecs_.find(kind);
	if (it != markerSpecs_.end()) {
		return it->second;
	}
	return MarkerSpec{};
}

bool MapRules::markerAllowedIn(const std::string& mapId,
			       MarkerKind kind) const {
	const MapRule* rule = find(mapId);
	if (rule == nullptr) {
		return true;  // los mapas extra admiten cualquier marcador
	}
	return std::find(rule->allowedMarkers.begin(),
			 rule->allowedMarkers.end(),
			 kind) != rule->allowedMarkers.end();
}

bool MapRules::canPlaceMarker(const Map& map, MarkerKind kind,
			      TilePos cell) const {
	const MarkerSpec spec = markerSpec(kind);
	return markerAllowedIn(map.id, kind) &&
	       map.fits(cell, 1, spec.height) &&
	       (!spec.requiresWalkable ||
		map.isAreaWalkable(cell, 1, spec.height));
}

bool MapRules::isMarkerUnique(const std::string& mapId, MarkerKind kind) const {
	const MapRule* rule = find(mapId);
	if (rule == nullptr) {
		return false;
	}
	return std::any_of(rule->requiredMarkers.begin(),
			   rule->requiredMarkers.end(),
			   [kind](const MarkerRule& req) {
				   return req.kind == kind && req.unique;
			   });
}

}
