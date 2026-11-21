#pragma once

#include <cstddef>
#include <string>

#include "world_model/Constants.h"
#include "world_model/PrefabCatalog.h"
#include "world_model/Scenario.h"
#include "world_model/ScenarioLibrary.h"
#include "world_model/World.h"
#include "world_model/WorldFactory.h"
#include "world_model/WorldSerializer.h"

namespace worldfixtures {

// Mundo mínimo pero completo: los mapas son chicos para que los arrays
// de 6 celdas se lean de un vistazo. Cubre un objeto, un marcador, un
// portal fijo, un mapa extra y un portal link.
inline const std::string kMinimalWorldToml = R"(format_version = 1

[world]
name = "Primavera"

[tilesets.outdoors]
image = "Farm/spring_outdoors.png"
cell = [16, 16]

[tilesets.interiors]
image = "Interiors/cabin.png"
cell = [16, 16]

[maps.farm]
kind = "farm"
width = 3
height = 2
tile_size = 16
base_collision = [true, true, false, false, true, false]
collision = [true, true, false, false, true, false]
ground = [
	"outdoors:0,0", "outdoors:1,0", "outdoors:0,0",
	"outdoors:0,0", "outdoors:0,0", "outdoors:0,0",
]
paths = [
	"", "", "",
	"", "outdoors:5,0", "",
]

[[maps.farm.objects]]
role = "cabin"
tile = "outdoors:10,0"
cell = [0, 0]
size = [2, 1]
blocked = true
door = [0, 1]

[[maps.farm.markers]]
kind = "bed"
cell = [2, 1]

[[maps.farm.portals]]
kind = "town_entrance"
cell = [2, 1]
target_map = "town"
target_cell = [0, 1]

[maps.town]
kind = "town"
width = 3
height = 2
tile_size = 16
base_collision = [false, false, false, false, false, false]
collision = [false, false, false, false, false, false]
ground = [
	"outdoors:0,0", "outdoors:0,0", "outdoors:0,0",
	"outdoors:0,0", "outdoors:0,0", "outdoors:0,0",
]

[[maps.town.portals]]
kind = "link"
cell = [0, 0]
target_map = "secret_cave"
target_cell = [0, 0]

[maps.dungeon_1]
kind = "dungeon"
level = 1
width = 3
height = 2
tile_size = 16
base_collision = [true, true, true, true, false, true]
collision = [true, true, true, true, false, true]
ground = [
	"interiors:0,0", "interiors:0,0", "interiors:0,0",
	"interiors:0,0", "interiors:0,0", "interiors:0,0",
]

[maps.secret_cave]
kind = "dungeon"
level = 3
width = 1
height = 1
tile_size = 16
base_collision = [false]
collision = [false]
ground = ["interiors:0,0"]
)";

inline worldmodel::World loadMinimalWorld() {
	return worldmodel::WorldSerializer().loadFromString(kMinimalWorldToml);
}

constexpr int kWidth = 48;
constexpr int kHeight = 20;

// Escenario liso, todo transitable, con un unico tileset "outdoors".
inline worldmodel::Scenario scenarioOf(const std::string& id,
				       worldmodel::MapKind kind,
				       int width = kWidth,
				       int height = kHeight) {
	worldmodel::Scenario scenario;
	scenario.id = id;
	scenario.kind = kind;
	scenario.width = width;
	scenario.height = height;
	scenario.tilesets.emplace("outdoors",
				  worldmodel::TilesetDef{"outdoors.png", 16, 16});
	const std::size_t cells = static_cast<std::size_t>(width) * height;
	scenario.collision.assign(cells, false);
	scenario.ground.assign(cells, worldmodel::TileRef{"outdoors", 0, 0});
	return scenario;
}

// Un escenario por tipo: farm_a, town_a, cabin_a, market_a, center_a y
// cave_a (este ultimo para las 10 mazmorras).
inline worldmodel::ScenarioLibrary fullLibrary() {
	worldmodel::ScenarioLibrary library;
	library.add(scenarioOf("farm_a", worldmodel::MapKind::Farm));
	library.add(scenarioOf("town_a", worldmodel::MapKind::Town));
	library.add(scenarioOf("cabin_a", worldmodel::MapKind::Cabin));
	library.add(scenarioOf("market_a", worldmodel::MapKind::Market));
	library.add(scenarioOf("center_a",
			       worldmodel::MapKind::CommunityCenter));
	library.add(scenarioOf("cave_a", worldmodel::MapKind::Dungeon));
	return library;
}

inline worldmodel::ScenarioChoice validChoice() {
	worldmodel::ScenarioChoice choice;
	choice.farm = "farm_a";
	choice.town = "town_a";
	choice.cabin = "cabin_a";
	choice.market = "market_a";
	choice.communityCenter = "center_a";
	choice.dungeons.fill("cave_a");
	return choice;
}

inline worldmodel::PrefabCatalog defaultCatalog() {
	worldmodel::PrefabCatalog catalog;
	catalog.setTilesets({
		{"prefab_estructuras_granja",
		 worldmodel::TilesetDef{
			 "World/prefabs/estructuras_granja.png", 16, 16}},
		{"prefab_casas_y_comercios",
		 worldmodel::TilesetDef{
			 "World/prefabs/casas_y_comercios.png", 16, 16}},
	});

	worldmodel::PrefabDef cabin;
	cabin.id = "casa_granja_1";
	cabin.name = "Casa de Granja (Nivel 1)";
	cabin.category = "estructuras_granja";
	cabin.categoryLabel = "Estructuras de granja";
	cabin.tile = worldmodel::TileRef{"prefab_estructuras_granja", 0, 0};
	cabin.width = 9;
	cabin.height = 9;
	cabin.role = worldmodel::ObjectRole::Cabin;
	cabin.door = worldmodel::TilePos{5, 7};
	cabin.collisionGrid.assign(81, true);
	cabin.collisionGrid[7 * 9 + 5] = false;
	catalog.add(cabin);

	worldmodel::PrefabDef center;
	center.id = "centro_comunitario";
	center.name = "Centro Comunitario (Restaurado)";
	center.category = "casas_y_comercios";
	center.categoryLabel = "Casas y comercios";
	center.tile = worldmodel::TileRef{"prefab_casas_y_comercios", 8, 20};
	center.width = 12;
	center.height = 10;
	center.role = worldmodel::ObjectRole::CommunityCenter;
	center.door = worldmodel::TilePos{5, 9};
	center.collisionGrid.assign(120, true);
	center.collisionGrid[9 * 12 + 5] = false;
	catalog.add(center);

	return catalog;
}

// Mundo "Primavera" valido (con 1 cabana, cabin_1 y portales externos) para tests.
inline worldmodel::World defaultWorld() {
	const worldmodel::ScenarioLibrary lib = fullLibrary();
	const worldmodel::PrefabCatalog cat = defaultCatalog();
	worldmodel::World world = worldmodel::WorldFactory().makeDefault(
		"Primavera", lib, cat, validChoice());
	const worldmodel::PrefabDef* cabin =
		cat.findByRole(worldmodel::ObjectRole::Cabin);
	worldmodel::Map& farm = world.maps.at("farm");
	world.tilesets.emplace(
		cabin->tile.tileset,
		cat.tilesets().at(cabin->tile.tileset));
	farm.objects.push_back(worldmodel::MapObject{
		worldmodel::ObjectRole::Cabin,
		cabin->tile,
		worldmodel::kFarmCabinOrigin,
		cabin->width,
		cabin->height,
		true,
		worldmodel::TilePos{
			worldmodel::kFarmCabinOrigin.column +
				cabin->door->column,
			worldmodel::kFarmCabinOrigin.row + cabin->door->row},
		cabin->collisionGrid,
	});
	farm.rebuildCollision();
	worldmodel::WorldFactory().addCabinMap(world, 1, *lib.find("cabin_a"));

	const worldmodel::TilePos farmTown{farm.width - 2, farm.height / 2};
	const worldmodel::TilePos farmDungeon{farm.width - 3, 2};
	worldmodel::Map& town = world.maps.at("town");
	const worldmodel::TilePos townFarm{1, town.height / 2};
	farm.portals.push_back(worldmodel::Portal{
		worldmodel::PortalKind::TownEntrance, farmTown, "town",
		townFarm});
	farm.portals.push_back(worldmodel::Portal{
		worldmodel::PortalKind::DungeonEntrance, farmDungeon,
		"dungeon_1", {2, 2}});
	town.portals.push_back(worldmodel::Portal{
		worldmodel::PortalKind::FarmEntrance, townFarm, "farm",
		farmTown});
	for (int level = 1; level <= worldmodel::kDungeonLevels; ++level) {
		worldmodel::Map& d =
			world.maps.at("dungeon_" + std::to_string(level));
		d.portals.push_back(worldmodel::Portal{
			worldmodel::PortalKind::DungeonExit, {2, 2}, "farm",
			farmDungeon});
		if (level < worldmodel::kDungeonLevels) {
			d.portals.push_back(worldmodel::Portal{
				worldmodel::PortalKind::StairsDown,
				{d.width - 3, d.height - 3},
				"dungeon_" + std::to_string(level + 1),
				{2, 2}});
		}
	}
	return world;
}

}
