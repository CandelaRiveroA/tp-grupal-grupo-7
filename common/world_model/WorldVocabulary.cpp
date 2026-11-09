#include "world_model/WorldVocabulary.h"

namespace worldmodel {

WorldVocabulary::WorldVocabulary() :
	mapKindTexts_{{MapKind::Farm, "farm"},
		      {MapKind::Town, "town"},
		      {MapKind::Cabin, "cabin"},
		      {MapKind::Market, "market"},
		      {MapKind::CommunityCenter, "community_center"},
		      {MapKind::Dungeon, "dungeon"}},
	objectRoleTexts_{{ObjectRole::Cabin, "cabin"},
			 {ObjectRole::Market, "market"},
			 {ObjectRole::CommunityCenter, "community_center"},
			 {ObjectRole::Decoration, "decoration"}},
	markerKindTexts_{{MarkerKind::PlayerSpawn, "player_spawn"},
			 {MarkerKind::Bed, "bed"},
			 {MarkerKind::MerchantMarnie, "merchant_marnie"},
			 {MarkerKind::MerchantClint, "merchant_clint"},
			 {MarkerKind::MerchantPierre, "merchant_pierre"},
			 {MarkerKind::MerchantRobin, "merchant_robin"}},
	portalKindTexts_{{PortalKind::TownEntrance, "town_entrance"},
			 {PortalKind::FarmEntrance, "farm_entrance"},
			 {PortalKind::DungeonEntrance, "dungeon_entrance"},
			 {PortalKind::StairsDown, "stairs_down"},
			 {PortalKind::DungeonExit, "dungeon_exit"},
			 {PortalKind::InteriorExit, "interior_exit"},
			 {PortalKind::Link, "link"}} {
	for (const auto& [kind, text] : mapKindTexts_) {
		mapKinds_.emplace(text, kind);
	}
	for (const auto& [role, text] : objectRoleTexts_) {
		objectRoles_.emplace(text, role);
	}
	for (const auto& [kind, text] : markerKindTexts_) {
		markerKinds_.emplace(text, kind);
	}
	for (const auto& [kind, text] : portalKindTexts_) {
		portalKinds_.emplace(text, kind);
	}
}

std::string WorldVocabulary::toText(MapKind kind) const {
	return mapKindTexts_.at(kind);
}

std::string WorldVocabulary::toText(ObjectRole role) const {
	return objectRoleTexts_.at(role);
}

std::string WorldVocabulary::toText(MarkerKind kind) const {
	return markerKindTexts_.at(kind);
}

std::string WorldVocabulary::toText(PortalKind kind) const {
	return portalKindTexts_.at(kind);
}

std::optional<MapKind> WorldVocabulary::mapKindFrom(
	const std::string& text) const {
	const auto it = mapKinds_.find(text);
	if (it == mapKinds_.end()) {
		return std::nullopt;
	}
	return it->second;
}

std::optional<ObjectRole> WorldVocabulary::objectRoleFrom(
	const std::string& text) const {
	const auto it = objectRoles_.find(text);
	if (it == objectRoles_.end()) {
		return std::nullopt;
	}
	return it->second;
}

std::optional<MarkerKind> WorldVocabulary::markerKindFrom(
	const std::string& text) const {
	const auto it = markerKinds_.find(text);
	if (it == markerKinds_.end()) {
		return std::nullopt;
	}
	return it->second;
}

std::optional<PortalKind> WorldVocabulary::portalKindFrom(
	const std::string& text) const {
	const auto it = portalKinds_.find(text);
	if (it == portalKinds_.end()) {
		return std::nullopt;
	}
	return it->second;
}

}
