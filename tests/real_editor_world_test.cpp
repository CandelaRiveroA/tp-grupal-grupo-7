#include <string>

#include "gtest/gtest.h"
#include "world/MapSceneAdapter.h"
#include "world/OverlayLoader.h"
#include "world/WorldMapLoader.h"
#include "world_model/PrefabCatalogLoader.h"
#include "world_model/ScenarioLibraryLoader.h"
#include "world_model/WorldFactory.h"
#include "world_model/WorldSerializer.h"

namespace {

#if defined(TALLER_EDITOR_SCENARIOS_DIR) && defined(TALLER_OVERLAY_TOML) && \
	defined(TALLER_PREFABS_TOML)

// Un mundo "como lo deja el editor": los escenarios reales de
// editor/scenarios armados por el WorldFactory del editor con sus edificios
// colocados, guardados con su writer y leídos por el camino del cliente.
worldmodel::World editorWorldRoundTrip() {
	const worldmodel::ScenarioLibrary library =
		worldmodel::ScenarioLibraryLoader().loadFromDirectory(
			TALLER_EDITOR_SCENARIOS_DIR);
	const worldmodel::PrefabCatalog catalog =
		worldmodel::PrefabCatalogLoader().loadFromFile(
			TALLER_PREFABS_TOML);
	const worldmodel::WorldFactory factory;
	worldmodel::World made = factory.makeDefault(
		"real", library, factory.firstOfEachKind(library));

	const auto place = [&](worldmodel::Map& target,
			       worldmodel::ObjectRole role,
			       worldmodel::TilePos origin) {
		const worldmodel::PrefabDef* prefab = catalog.findByRole(role);
		made.tilesets.emplace(prefab->tile.tileset,
				      catalog.tilesets().at(prefab->tile.tileset));
		std::optional<worldmodel::TilePos> door;
		if (prefab->door.has_value()) {
			door = worldmodel::TilePos{
				origin.column + prefab->door->column,
				origin.row + prefab->door->row};
		}
		target.objects.push_back(worldmodel::MapObject{
			role, prefab->tile, origin, prefab->width, prefab->height,
			true, door, prefab->collisionGrid, prefab->doorWidth});
	};

	place(made.maps.at("farm"), worldmodel::ObjectRole::Cabin, {10, 10});
	place(made.maps.at("farm"), worldmodel::ObjectRole::Barn, {25, 10});
	made.maps.at("farm").rebuildCollision();
	factory.addCabinMap(
		made, 1,
		*library.find(factory.firstOfEachKind(library).cabin));

	worldmodel::Map& town = made.maps.at("town");
	const worldmodel::ObjectRole shopRoles[] = {
		worldmodel::ObjectRole::Carpentry,
		worldmodel::ObjectRole::Blacksmith,
		worldmodel::ObjectRole::GeneralStore,
		worldmodel::ObjectRole::Ranch,
	};
	int col = 5;
	for (const worldmodel::ObjectRole role : shopRoles) {
		place(town, role, {col, 5});
		col += 18;
	}
	place(town, worldmodel::ObjectRole::CommunityCenter, {10, 25});
	factory.wirePortals(made);

	return world::WorldMapLoader().loadFromString(
		worldmodel::WorldSerializer().saveToString(made));
}

TEST(RealEditorWorldTest, EveryMapOfTheEditorWorldBecomesAScene) {
	const worldmodel::World editorWorld = editorWorldRoundTrip();
	const world::WorldOverlay overlay =
		world::OverlayLoader().loadFromFile(TALLER_OVERLAY_TOML);
	ASSERT_EQ(editorWorld.maps.size(), 18u);
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
		// Todos los mapas traen piso y capa frontal.
		ASSERT_GE(scene.layers.size(), 2u) << entry.first;
		EXPECT_EQ(scene.layers.front().pass,
			  world::LayerPass::Background)
			<< entry.first;
		EXPECT_EQ(scene.layers.back().pass,
			  world::LayerPass::Foreground)
			<< entry.first;
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
	// Los 4 comercios y el centro comunitario traen comportamiento del
	// overlay real.
	EXPECT_EQ(withAction, 5);
}

TEST(RealEditorWorldTest, ShopMerchantsGetTheirCellFromTheEditorMarkers) {
	const worldmodel::World editorWorld = editorWorldRoundTrip();
	const world::WorldOverlay overlay =
		world::OverlayLoader().loadFromFile(TALLER_OVERLAY_TOML);
	const std::pair<const char*, const char*> shops[] = {
		{"general_store", "pierre"},
		{"ranch", "marnie"},
		{"blacksmith", "clint"},
		{"carpentry", "robin"},
	};
	for (const auto& [mapId, npc] : shops) {
		const world::SceneDescription shop =
			world::MapSceneAdapter(overlay).adapt(editorWorld,
							      mapId);
		EXPECT_EQ(shop.npcCells.count(npc), 1u) << mapId << ":" << npc;
	}
}

#endif

}  // namespace
