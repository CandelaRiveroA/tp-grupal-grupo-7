#include <string>

#include "gtest/gtest.h"
#include "world/MapSceneAdapter.h"
#include "world/OverlayLoader.h"
#include "world/WorldMapLoader.h"
#include "world_model/ScenarioLibraryLoader.h"
#include "world_model/WorldFactory.h"
#include "world_model/WorldSerializer.h"

namespace {

#if defined(TALLER_EDITOR_SCENARIOS_DIR) && defined(TALLER_OVERLAY_TOML)

// Un mundo "como lo deja el editor": los escenarios reales de
// editor/scenarios armados por el WorldFactory del editor, guardados con
// su writer y leídos por el camino del cliente.
worldmodel::World editorWorldRoundTrip() {
	const worldmodel::ScenarioLibrary library =
		worldmodel::ScenarioLibraryLoader().loadFromDirectory(
			TALLER_EDITOR_SCENARIOS_DIR);
	const worldmodel::WorldFactory factory;
	const worldmodel::World made = factory.makeDefault(
		"real", library, factory.firstOfEachKind(library));
	return world::WorldMapLoader().loadFromString(
		worldmodel::WorldSerializer().saveToString(made));
}

TEST(RealEditorWorldTest, EveryMapOfTheEditorWorldBecomesAScene) {
	const worldmodel::World editorWorld = editorWorldRoundTrip();
	const world::WorldOverlay overlay =
		world::OverlayLoader().loadFromFile(TALLER_OVERLAY_TOML);
	ASSERT_EQ(editorWorld.maps.size(), 15u);
	for (const auto& entry : editorWorld.maps) {
		const worldmodel::Map& map = entry.second;
		const world::SceneDescription scene =
			world::MapSceneAdapter(overlay).adapt(editorWorld,
							      entry.first);
		// Coherencia en celdas: la escena mide lo que el mapa, con una
		// celda de tile_size * scale px.
		EXPECT_EQ(scene.grid.columns, map.width) << entry.first;
		EXPECT_EQ(scene.grid.rows, map.height) << entry.first;
		EXPECT_EQ(scene.cellPixelSize, map.tileSize * overlay.scale)
			<< entry.first;
		// Todos los mapas traen su piso.
		EXPECT_FALSE(scene.layers.empty()) << entry.first;
		EXPECT_EQ(scene.layers[0].cells.size(),
			  static_cast<std::size_t>(map.width * map.height))
			<< entry.first;
		EXPECT_EQ(scene.elements.size(), map.objects.size())
			<< entry.first;
	}
}

TEST(RealEditorWorldTest, TownBuildingsBecomeClickableStructures) {
	const worldmodel::World editorWorld = editorWorldRoundTrip();
	const world::WorldOverlay overlay =
		world::OverlayLoader().loadFromFile(TALLER_OVERLAY_TOML);
	const world::SceneDescription town =
		world::MapSceneAdapter(overlay).adapt(editorWorld, "town");
	int withAction = 0;
	for (const world::SceneStructure& structure : town.structures) {
		withAction += structure.action != world::StructureAction::None;
	}
	// El mercado y el centro comunitario traen comportamiento del
	// overlay real.
	EXPECT_EQ(withAction, 2);
}

TEST(RealEditorWorldTest, MarketMerchantsGetTheirCellFromTheEditorMarkers) {
	const worldmodel::World editorWorld = editorWorldRoundTrip();
	const world::WorldOverlay overlay =
		world::OverlayLoader().loadFromFile(TALLER_OVERLAY_TOML);
	const world::SceneDescription market =
		world::MapSceneAdapter(overlay).adapt(editorWorld, "market");
	for (const char* npc : {"pierre", "marnie", "clint", "robin"}) {
		EXPECT_EQ(market.npcCells.count(npc), 1u) << npc;
	}
}

#endif

}  // namespace
