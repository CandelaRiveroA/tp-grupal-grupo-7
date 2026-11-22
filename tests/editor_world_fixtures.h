#pragma once

#include <string>
#include <utility>

#include "world_model/WorldSerializer.h"

// Mundo mínimo con el MISMO formato que guarda el editor: se arma con el
// modelo y se serializa con el writer real, así el fixture nunca se
// desvía del formato. Los tests lo cargan por el camino del cliente.
namespace editor_world_fixtures {

inline worldmodel::TileRef tile(const std::string& tileset, int column,
				int row) {
	worldmodel::TileRef ref;
	ref.tileset = tileset;
	ref.column = column;
	ref.row = row;
	return ref;
}

inline worldmodel::MapObject object(worldmodel::ObjectRole role,
				    const worldmodel::TileRef& tileRef,
				    int column, int row, int width,
				    int height) {
	worldmodel::MapObject entry;
	entry.role = role;
	entry.tile = tileRef;
	entry.cell = {column, row};
	entry.width = width;
	entry.height = height;
	return entry;
}

inline worldmodel::Map emptyMap(const std::string& id,
				worldmodel::MapKind kind, int width,
				int height) {
	worldmodel::Map map;
	map.id = id;
	map.kind = kind;
	map.width = width;
	map.height = height;
	map.tileSize = 16;
	const std::size_t cells = static_cast<std::size_t>(width * height);
	map.baseCollision.assign(cells, false);
	map.collision.assign(cells, false);
	map.ground.assign(cells, std::nullopt);
	map.paths.assign(cells, std::nullopt);
	return map;
}

// "town" 6x5: piso con una celda vacía en (0,0), un camino en (1,1),
// un árbol (1x2), el mercado (2x2, puerta en (2,3)) y el centro
// comunitario (2x2, puerta en (5,3)). "market" 5x4: un comerciante.
// "cabin" 4x4: la cama.
inline worldmodel::World makeWorld() {
	worldmodel::World world;
	world.name = "fixture";
	world.tilesets["scene_a"] = {"World/scenarios/a.png", 16, 16};
	world.tilesets["prefab_b"] = {"World/prefabs/b.png", 16, 16};

	worldmodel::Map town =
		emptyMap("town", worldmodel::MapKind::Town, 6, 5);
	for (std::size_t i = 1; i < town.ground.size(); ++i) {
		town.ground[i] = tile("scene_a", 0, 0);
	}
	town.paths[town.cellIndex({1, 1})] = tile("scene_a", 3, 2);
	town.objects.push_back(object(worldmodel::ObjectRole::Decoration,
				      tile("prefab_b", 0, 0), 4, 0, 1, 2));
	worldmodel::MapObject market =
		object(worldmodel::ObjectRole::Market, tile("prefab_b", 2, 0),
		       1, 1, 2, 2);
	market.blocked = true;
	market.door = worldmodel::TilePos{2, 3};
	town.objects.push_back(market);
	worldmodel::MapObject center = object(
		worldmodel::ObjectRole::CommunityCenter, tile("prefab_b", 4, 0),
		4, 3, 2, 2);
	center.door = worldmodel::TilePos{5, 3};
	town.objects.push_back(center);
	town.portals.push_back({worldmodel::PortalKind::FarmEntrance,
				{0, 4},
				"town",
				{0, 4}});
	world.maps["town"] = town;

	worldmodel::Map market2 =
		emptyMap("market", worldmodel::MapKind::Market, 5, 4);
	for (auto& cell : market2.ground) {
		cell = tile("scene_a", 1, 1);
	}
	market2.markers.push_back({worldmodel::MarkerKind::MerchantPierre,
				   {2, 2}});
	world.maps["market"] = market2;

	worldmodel::Map cabin =
		emptyMap("cabin", worldmodel::MapKind::Cabin, 4, 4);
	for (auto& cell : cabin.ground) {
		cell = tile("scene_a", 2, 2);
	}
	cabin.markers.push_back({worldmodel::MarkerKind::Bed, {1, 1}});
	world.maps["cabin"] = cabin;
	return world;
}

inline std::string makeToml() {
	return worldmodel::WorldSerializer().saveToString(makeWorld());
}

}  // namespace editor_world_fixtures
