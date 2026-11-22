#include "world_model/MapRules.h"

#include <string>
#include <vector>

namespace worldmodel {

MapRules::MapRules() {
	rules_.reserve(15);
	const auto add = [this](const std::string& id, MapKind kind) -> MapRule& {
		rules_.push_back(MapRule{});
		MapRule& rule = rules_.back();
		rule.id = id;
		rule.kind = kind;
		return rule;
	};

	const std::vector<MarkerKind> merchants{
		MarkerKind::MerchantMarnie, MarkerKind::MerchantClint,
		MarkerKind::MerchantPierre, MarkerKind::MerchantRobin};

	MapRule& farm = add("farm", MapKind::Farm);
	farm.allowedMarkers = {MarkerKind::PlayerSpawn};
	farm.requiredMarkers = {{MarkerKind::PlayerSpawn, false}};
	farm.requiredObjects = {ObjectRole::Cabin};
	farm.requiredPortals = {PortalKind::TownEntrance,
				PortalKind::DungeonEntrance};

	MapRule& town = add("town", MapKind::Town);
	town.requiredObjects = {ObjectRole::Market,
				ObjectRole::CommunityCenter};
	town.requiredPortals = {PortalKind::FarmEntrance};

	MapRule& cabin = add("cabin", MapKind::Cabin);
	cabin.allowedMarkers = {MarkerKind::Bed};
	cabin.requiredMarkers = {{MarkerKind::Bed, false}};
	cabin.requiredPortals = {PortalKind::InteriorExit};
	cabin.interiorObject = ObjectRole::Cabin;
	cabin.interiorExitTarget = "farm";

	MapRule& market = add("market", MapKind::Market);
	market.allowedMarkers = merchants;
	market.requiredMarkers = {{MarkerKind::MerchantMarnie, true},
				  {MarkerKind::MerchantClint, true},
				  {MarkerKind::MerchantPierre, true},
				  {MarkerKind::MerchantRobin, true}};
	market.requiredPortals = {PortalKind::InteriorExit};
	market.interiorObject = ObjectRole::Market;
	market.interiorExitTarget = "town";

	MapRule& center = add("community_center", MapKind::CommunityCenter);
	center.requiredPortals = {PortalKind::InteriorExit};
	center.interiorObject = ObjectRole::CommunityCenter;
	center.interiorExitTarget = "town";

	for (int level = 1; level <= 10; ++level) {
		MapRule& dungeon = add(dungeonId(level), MapKind::Dungeon);
		dungeon.requiredPortals = {PortalKind::DungeonExit};
		if (level < 10) {
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

std::string MapRules::dungeonId(int level) const {
	return "dungeon_" + std::to_string(level);
}

std::optional<int> MapRules::dungeonLevel(const std::string& id) const {
	for (int level = 1; level <= 10; ++level) {
		if (id == dungeonId(level)) {
			return level;
		}
	}
	return std::nullopt;
}

}
