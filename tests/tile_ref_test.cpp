#include "gtest/gtest.h"
#include "world_model/Tile.h"

namespace {

TEST(TileRefTest, KeyEncodesTilesetColumnRow) {
	const worldmodel::TileRef tile{"outdoors", 3, 2};
	EXPECT_EQ(tile.key(), "outdoors:3,2");
}

TEST(TileRefTest, EqualityComparesAllFields) {
	EXPECT_EQ((worldmodel::TileRef{"outdoors", 3, 2}),
		  (worldmodel::TileRef{"outdoors", 3, 2}));
	EXPECT_NE((worldmodel::TileRef{"outdoors", 3, 2}),
		  (worldmodel::TileRef{"outdoors", 3, 3}));
}

TEST(TilesetDefTest, EqualityComparesAllFields) {
	EXPECT_EQ((worldmodel::TilesetDef{"outdoors.png", 16, 16}),
		  (worldmodel::TilesetDef{"outdoors.png", 16, 16}));
	EXPECT_NE((worldmodel::TilesetDef{"outdoors.png", 16, 16}),
		  (worldmodel::TilesetDef{"outdoors.png", 16, 32}));
}

}
