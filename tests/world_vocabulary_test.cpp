#include <string>
#include <utility>

#include "gtest/gtest.h"
#include "world_model/WorldVocabulary.h"

namespace {

const worldmodel::WorldVocabulary kVocabulary;

TEST(WorldVocabularyTest, MapKindRoundTrip) {
	const std::pair<std::string, worldmodel::MapKind> cases[] = {
		{"farm", worldmodel::MapKind::Farm},
		{"town", worldmodel::MapKind::Town},
		{"cabin", worldmodel::MapKind::Cabin},
		{"carpentry", worldmodel::MapKind::Carpentry},
		{"blacksmith", worldmodel::MapKind::Blacksmith},
		{"general_store", worldmodel::MapKind::GeneralStore},
		{"ranch", worldmodel::MapKind::Ranch},
		{"dungeon", worldmodel::MapKind::Dungeon},
	};
	for (const auto& [text, kind] : cases) {
		EXPECT_EQ(kVocabulary.toText(kind), text);
		EXPECT_EQ(kVocabulary.mapKindFrom(text), kind);
	}
}

TEST(WorldVocabularyTest, ObjectRoleRoundTrip) {
	const std::pair<std::string, worldmodel::ObjectRole> cases[] = {
		{"cabin", worldmodel::ObjectRole::Cabin},
		{"carpentry", worldmodel::ObjectRole::Carpentry},
		{"blacksmith", worldmodel::ObjectRole::Blacksmith},
		{"general_store", worldmodel::ObjectRole::GeneralStore},
		{"ranch", worldmodel::ObjectRole::Ranch},
		{"community_center", worldmodel::ObjectRole::CommunityCenter},
		{"decoration", worldmodel::ObjectRole::Decoration},
	};
	for (const auto& [text, role] : cases) {
		EXPECT_EQ(kVocabulary.toText(role), text);
		EXPECT_EQ(kVocabulary.objectRoleFrom(text), role);
	}
}

TEST(WorldVocabularyTest, MarkerKindRoundTrip) {
	const std::pair<std::string, worldmodel::MarkerKind> cases[] = {
		{"bed", worldmodel::MarkerKind::Bed},
		{"merchant_marnie", worldmodel::MarkerKind::MerchantMarnie},
		{"merchant_clint", worldmodel::MarkerKind::MerchantClint},
		{"merchant_pierre", worldmodel::MarkerKind::MerchantPierre},
		{"merchant_robin", worldmodel::MarkerKind::MerchantRobin},
	};
	for (const auto& [text, kind] : cases) {
		EXPECT_EQ(kVocabulary.toText(kind), text);
		EXPECT_EQ(kVocabulary.markerKindFrom(text), kind);
	}
}

TEST(WorldVocabularyTest, PortalKindRoundTrip) {
	const std::pair<std::string, worldmodel::PortalKind> cases[] = {
		{"town_entrance", worldmodel::PortalKind::TownEntrance},
		{"farm_entrance", worldmodel::PortalKind::FarmEntrance},
		{"dungeon_entrance", worldmodel::PortalKind::DungeonEntrance},
		{"stairs_down", worldmodel::PortalKind::StairsDown},
		{"dungeon_exit", worldmodel::PortalKind::DungeonExit},
		{"interior_exit", worldmodel::PortalKind::InteriorExit},
		{"link", worldmodel::PortalKind::Link},
	};
	for (const auto& [text, kind] : cases) {
		EXPECT_EQ(kVocabulary.toText(kind), text);
		EXPECT_EQ(kVocabulary.portalKindFrom(text), kind);
	}
}

TEST(WorldVocabularyTest, UnknownTextsGiveNullopt) {
	EXPECT_FALSE(kVocabulary.mapKindFrom("nope").has_value());
	EXPECT_FALSE(kVocabulary.objectRoleFrom("nope").has_value());
	EXPECT_FALSE(kVocabulary.markerKindFrom("nope").has_value());
	EXPECT_FALSE(kVocabulary.portalKindFrom("nope").has_value());
}

}
