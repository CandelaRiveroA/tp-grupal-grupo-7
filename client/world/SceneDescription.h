#pragma once

#include <string>
#include <unordered_map>
#include <vector>

#include "world/Color.h"
#include "world/Grid.h"

namespace world {


struct TileSetDef {
	std::string image;
	int cellWidth;
	int cellHeight;
};

struct SpriteRegion {
	GridCell origin;
	GridSpan size;
};

enum class DrawMode {
	Single,
	Tile,
	HorizontalStrip,
};

struct DrawSpec {
	DrawMode mode;
	int scaleX;
	int scaleY;
};

struct ElementPlacement {
	std::string tileset;
	SpriteRegion region;
	DrawSpec draw;
	GridCell at;
	GridSpan span;
};

struct SceneDescription {
	Color floorColor;
	GridSpan grid;
	int cellPixelSize;
	std::unordered_map<std::string, TileSetDef> tilesets;
	std::vector<ElementPlacement> elements;
};

}
