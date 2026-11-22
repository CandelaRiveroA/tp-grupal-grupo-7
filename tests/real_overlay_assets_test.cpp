#include <algorithm>

#include "editor_world_fixtures.h"
#include "gtest/gtest.h"
#include "world/NpcCatalog.h"
#include "world/OverlayAudit.h"
#include "world/OverlayLoader.h"

namespace {

#if defined(TALLER_OVERLAY_TOML) && defined(TALLER_NPCS_TOML)

// Los archivos REALES que usa el juego: un typo en overlay.toml o en el
// popup del centro comunitario se detecta acá y no al jugar.
TEST(RealOverlayAssetsTest, OverlayParsesWithTheExpectedBehaviors) {
	const world::WorldOverlay overlay =
		world::OverlayLoader().loadFromFile(TALLER_OVERLAY_TOML);
	EXPECT_EQ(overlay.scale, 2);
	ASSERT_EQ(overlay.roles.count("community_center"), 1u);
	EXPECT_EQ(overlay.roles.at("community_center").action,
		  world::StructureAction::Popup);
	ASSERT_EQ(overlay.roles.count("market"), 1u);
	EXPECT_EQ(overlay.roles.at("market").action,
		  world::StructureAction::EnterMap);
	EXPECT_EQ(overlay.markerNpcs.at("merchant_pierre"), "pierre");
}

TEST(RealOverlayAssetsTest, EveryNpcAndPopupTheOverlayNamesExists) {
	const world::WorldOverlay overlay =
		world::OverlayLoader().loadFromFile(TALLER_OVERLAY_TOML);
	const world::NpcCatalog npcs(TALLER_NPCS_TOML);
	for (const auto& entry : overlay.markerNpcs) {
		EXPECT_NE(npcs.find(entry.second), nullptr) << entry.second;
	}
	for (const auto& entry : overlay.roles) {
		if (entry.second.action == world::StructureAction::Popup) {
			EXPECT_NE(npcs.find(entry.second.target), nullptr)
				<< entry.second.target;
		}
	}
}

TEST(RealOverlayAssetsTest, NothingInTheOverlayIsUnmatchedByTheVocabulary) {
	const world::WorldOverlay overlay =
		world::OverlayLoader().loadFromFile(TALLER_OVERLAY_TOML);
	EXPECT_TRUE(world::OverlayAudit()
			    .unmatched(editor_world_fixtures::makeWorld(), overlay)
			    .empty());
}

TEST(RealOverlayAssetsTest, PopupOnlyEntryNeverShowsUpInAScene) {
	const world::NpcCatalog npcs(TALLER_NPCS_TOML);
	const world::NpcDef* popup = npcs.find("community_center");
	ASSERT_NE(popup, nullptr);
	for (const char* room : {"market", "bedroom", "town", "farm"}) {
		const std::vector<const world::NpcDef*> inRoom = npcs.forRoom(room);
		EXPECT_EQ(std::count(inRoom.begin(), inRoom.end(), popup), 0)
			<< room;
	}
}

#endif

}  // namespace
