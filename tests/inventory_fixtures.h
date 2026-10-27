#pragma once

#include <string>

#include "world/InventoryConfigLoader.h"

namespace inventory_fixtures {

inline const std::string kBarToml = R"(
[sheet]
image = "Players/main/tools/MenuTiles.png"
cell  = [16, 16]

[slot]
cell = [4, 8]

[bar]
count  = 9
scale  = 3
gap    = 0
anchor = "bottom_center"
margin = 16
)";

inline world::InventoryLayout loadString(const std::string& toml) {
	world::InventoryConfigLoader loader;
	return loader.loadFromString(toml);
}

inline world::InventoryLayout loadBar() { return loadString(kBarToml); }

inline const std::string kBarWithFrameToml = R"(
[sheet]
image = "Players/main/tools/MenuTiles.png"
cell  = [16, 16]

[slot]
cell = [4, 8]

[bar]
count  = 9
scale  = 4
gap    = 0
anchor = "bottom_center"
margin = 16

[frame]
border_cells = 2
thickness    = 16
top_left     = [1, 1]
top_right    = [13, 1]
bottom_left  = [1, 13]
bottom_right = [13, 13]
top          = [3, 1]
bottom       = [3, 13]
left         = [1, 3]
right        = [13, 3]

[slot_frame]
border_cells = 1
thickness    = 4
top_left     = [8, 8]
top_right    = [11, 8]
bottom_left  = [8, 11]
bottom_right = [11, 11]
top          = [9, 8]
bottom       = [9, 11]
left         = [8, 10]
right        = [11, 10]
)";

inline world::InventoryLayout loadBarWithFrame() {
	return loadString(kBarWithFrameToml);
}

}
