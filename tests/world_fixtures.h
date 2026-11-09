#pragma once

#include <string>

#include "world_model/World.h"
#include "world_model/WorldSerializer.h"

namespace worldfixtures {

// Mundo mínimo pero completo: los mapas son chicos para que los arrays
// de 6 celdas se lean de un vistazo. Cubre un objeto, un marcador, un
// portal fijo, un mapa extra y un portal link.
inline const std::string kMinimalWorldToml = R"(format_version = 1

[world]
slug = "spring"
name = "Primavera"
description = "Mundo de fixture"

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
kind = "player_spawn"
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

}
