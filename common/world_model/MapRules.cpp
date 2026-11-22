#include "world_model/MapRules.h"

#include <map>
#include <string>
#include <vector>

namespace worldmodel {

MapRules::MapRules() {
	rules_.reserve(kBaseMandatoryMaps + kMaxPlayers + kDungeonLevels);
	const auto add = [this](const std::string& id, MapKind kind) -> MapRule& {
		rules_.push_back(MapRule{});
		MapRule& rule = rules_.back();
		rule.id = id;
		rule.kind = kind;
		return rule;
	};

	MapRule& farm = add("farm", MapKind::Farm);
	farm.requiredPortals = {PortalKind::TownEntrance,
				PortalKind::DungeonEntrance};

	MapRule& town = add("town", MapKind::Town);
	town.requiredObjects = {ObjectRole::Carpentry, ObjectRole::Blacksmith,
				ObjectRole::GeneralStore, ObjectRole::Ranch,
				ObjectRole::CommunityCenter};
	town.requiredPortals = {PortalKind::FarmEntrance};

	for (int k = kMinPlayers; k <= kMaxPlayers; ++k) {
		MapRule& cabin = add(cabinId(k), MapKind::Cabin);
		cabin.allowedMarkers = {MarkerKind::Bed};
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
		shop.allowedMarkers = {spec.merchant};
		shop.requiredMarkers = {{spec.merchant, true}};
		shop.requiredPortals = {PortalKind::InteriorExit};
		shop.interiorObject = spec.role;
		shop.interiorExitTarget = "town";
	}

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
	std::vector<std::string> ids = {"farm",         "town",
					"carpentry",    "blacksmith",
					"general_store", "ranch"};
	for (int k = kMinPlayers; k <= cabinCount && k <= kMaxPlayers; ++k) {
		ids.push_back(cabinId(k));
	}
	for (int level = 1; level <= kDungeonLevels; ++level) {
		ids.push_back(dungeonId(level));
	}
	return ids;
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

}
