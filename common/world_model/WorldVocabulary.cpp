#include "world_model/WorldVocabulary.h"

#include <algorithm>
#include <utility>

namespace worldmodel {

namespace {

// Fuente única de verdad: cada par texto <-> enum se escribe una sola vez.
const std::pair<MapKind, const char*> kMapKinds[] = {
	{MapKind::Farm, "farm"},
	{MapKind::Town, "town"},
	{MapKind::Cabin, "cabin"},
	{MapKind::Market, "market"},
	{MapKind::CommunityCenter, "community_center"},
	{MapKind::Dungeon, "dungeon"},
};

const std::pair<ObjectRole, const char*> kObjectRoles[] = {
	{ObjectRole::Cabin, "cabin"},
	{ObjectRole::Market, "market"},
	{ObjectRole::CommunityCenter, "community_center"},
	{ObjectRole::Decoration, "decoration"},
};

const std::pair<MarkerKind, const char*> kMarkerKinds[] = {
	{MarkerKind::PlayerSpawn, "player_spawn"},
	{MarkerKind::Bed, "bed"},
	{MarkerKind::MerchantMarnie, "merchant_marnie"},
	{MarkerKind::MerchantClint, "merchant_clint"},
	{MarkerKind::MerchantPierre, "merchant_pierre"},
	{MarkerKind::MerchantRobin, "merchant_robin"},
};

const std::pair<PortalKind, const char*> kPortalKinds[] = {
	{PortalKind::TownEntrance, "town_entrance"},
	{PortalKind::FarmEntrance, "farm_entrance"},
	{PortalKind::DungeonEntrance, "dungeon_entrance"},
	{PortalKind::StairsDown, "stairs_down"},
	{PortalKind::DungeonExit, "dungeon_exit"},
	{PortalKind::InteriorExit, "interior_exit"},
	{PortalKind::Link, "link"},
};

template <typename Enum, std::size_t N>
std::string toText(Enum kind, const std::pair<Enum, const char*> (&table)[N]) {
	const auto found = std::find_if(std::begin(table), std::end(table),
					[kind](const auto& entry) {
						return entry.first == kind;
					});
	return found != std::end(table) ? found->second : std::string();
}

template <typename Enum, std::size_t N>
std::optional<Enum> fromText(
	const std::string& text,
	const std::pair<Enum, const char*> (&table)[N]) {
	const auto found = std::find_if(
		std::begin(table), std::end(table),
		[&text](const auto& entry) { return text == entry.second; });
	return found != std::end(table) ? std::optional<Enum>(found->first)
					: std::nullopt;
}

}

std::string WorldVocabulary::toText(MapKind kind) const {
	return worldmodel::toText(kind, kMapKinds);
}

std::string WorldVocabulary::toText(ObjectRole role) const {
	return worldmodel::toText(role, kObjectRoles);
}

std::string WorldVocabulary::toText(MarkerKind kind) const {
	return worldmodel::toText(kind, kMarkerKinds);
}

std::string WorldVocabulary::toText(PortalKind kind) const {
	return worldmodel::toText(kind, kPortalKinds);
}

std::optional<MapKind> WorldVocabulary::mapKindFrom(
	const std::string& text) const {
	return worldmodel::fromText(text, kMapKinds);
}

std::optional<ObjectRole> WorldVocabulary::objectRoleFrom(
	const std::string& text) const {
	return worldmodel::fromText(text, kObjectRoles);
}

std::optional<MarkerKind> WorldVocabulary::markerKindFrom(
	const std::string& text) const {
	return worldmodel::fromText(text, kMarkerKinds);
}

std::optional<PortalKind> WorldVocabulary::portalKindFrom(
	const std::string& text) const {
	return worldmodel::fromText(text, kPortalKinds);
}

}
