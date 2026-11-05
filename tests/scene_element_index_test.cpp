#include <cstddef>
#include <cstdint>
#include <vector>

#include "gtest/gtest.h"
#include "world/SceneElementIndex.h"

namespace {

using world::Color;
using world::DrawMode;
using world::DrawSpec;
using world::ElementPlacement;
using world::GridCell;
using world::GridSpan;
using world::PixelRect;
using world::SceneDescription;
using world::SceneElementIndex;
using world::ScenePixelMapper;
using world::TileSetDef;

ElementPlacement tileElement(int column, int row) {
	return ElementPlacement{
		"tiles",
		GridSpan{1, 1},
		{{0, 0}},
		DrawSpec{DrawMode::Tile, 1, 1},
		GridCell{column, row},
		GridSpan{2, 2},
		std::nullopt,
	};
}

TEST(SceneElementIndexTest, QueryReturnsOnlyVisibleElementsInSceneOrder) {
	SceneDescription scene{Color{0, 0, 0, 255}, GridSpan{100, 100}, 32,
			       {}, {}, {}, {}};
	scene.tilesets["tiles"] = TileSetDef{"sheet.png", 16, 16};
	scene.elements.push_back(tileElement(0, 0));
	scene.elements.push_back(tileElement(50, 50));

	SceneElementIndex index;
	index.rebuild(scene, ScenePixelMapper{32});

	std::vector<std::uint32_t> visible;
	index.query(PixelRect{0, 0, 64, 64}, visible);

	ASSERT_EQ(visible.size(), 1u);
	EXPECT_EQ(visible[0], 0u);
}

TEST(SceneElementIndexTest, RebuildReplacesThePreviousIndex) {
	SceneDescription scene{Color{0, 0, 0, 255}, GridSpan{100, 100}, 32,
			       {}, {}, {}, {}};
	scene.tilesets["tiles"] = TileSetDef{"sheet.png", 16, 16};
	scene.elements.push_back(tileElement(50, 50));

	SceneElementIndex index;
	index.rebuild(scene, ScenePixelMapper{32});
	index.rebuild(scene, ScenePixelMapper{32});

	std::vector<std::uint32_t> visible;
	index.query(PixelRect{1600, 1600, 64, 64}, visible);

	ASSERT_EQ(visible.size(), 1u);
	EXPECT_EQ(visible[0], 0u);
}

}  // namespace
