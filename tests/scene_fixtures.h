#pragma once

#include <string>

#include "world/SceneConfigLoader.h"

namespace scene_fixtures {

inline const std::string kBedroomToml = R"(
[tilesets.farm]
image = "Farm/walls&floors.png"
cell  = [16, 16]

[rooms.bedroom]
floor_color = [0, 0, 0, 255]
grid        = [15, 10]
scale       = 2

  [[rooms.bedroom.elements]]
  tileset = "farm"
  region  = [0, 27, 2, 2]
  draw    = { mode = "tile" }
  at      = [0, 2]
  span    = [15, 8]

  [[rooms.bedroom.elements]]
  tileset = "farm"
  region  = [0, 0, 1, 3]
  draw    = { mode = "horizontal_strip", scale_x = 2, scale_y = 3 }
  at      = [0, 0]
  span    = [15, 2]
)";

inline world::SceneDescription loadString(const std::string& toml,
					  const std::string& room) {
	world::SceneConfigLoader loader;
	return loader.loadFromString(toml, room);
}

inline world::SceneDescription loadBedroom() {
	return loadString(kBedroomToml, "bedroom");
}

}
