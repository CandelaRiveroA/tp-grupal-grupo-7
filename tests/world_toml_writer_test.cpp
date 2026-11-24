#include <string>

#include "gtest/gtest.h"
#include "toml/TomlDocument.h"
#include "world_fixtures.h"
#include "world_model/WorldTomlWriter.h"

namespace {

using worldmodel::World;
using worldmodel::WorldTomlWriter;

tomlcfg::TomlDocument written(const World& world) {
	return tomlcfg::TomlDocument(WorldTomlWriter().write(world),
				     tomlcfg::TomlDocument::Source::Text);
}

TEST(WorldTomlWriterTest, WritesHeaderTilesetsAndMaps) {
	const std::string text =
		WorldTomlWriter().write(worldfixtures::loadMinimalWorld());
	EXPECT_NE(text.find("format_version = 1"), std::string::npos);
	EXPECT_NE(text.find("[world]"), std::string::npos);
	EXPECT_NE(text.find("[tilesets.outdoors]"), std::string::npos);
	EXPECT_NE(text.find("[maps.farm]"), std::string::npos);
}

TEST(WorldTomlWriterTest, AlwaysWritesCollision) {
	const tomlcfg::TomlDocument doc =
		written(worldfixtures::loadMinimalWorld());
	EXPECT_TRUE(doc.hasKey("maps.farm.base_collision"));
	EXPECT_TRUE(doc.hasKey("maps.farm.collision"));
}

TEST(WorldTomlWriterTest, WritesPathsOnlyWhenNotEmpty) {
	const tomlcfg::TomlDocument doc =
		written(worldfixtures::loadMinimalWorld());
	EXPECT_TRUE(doc.hasKey("maps.farm.paths"));
	EXPECT_FALSE(doc.hasKey("maps.town.paths"));
}

TEST(WorldTomlWriterTest, OmitsEmptySections) {
	const tomlcfg::TomlDocument doc =
		written(worldfixtures::loadMinimalWorld());
	EXPECT_FALSE(doc.hasKey("maps.secret_cave.objects"));
	EXPECT_FALSE(doc.hasKey("maps.secret_cave.markers"));
	EXPECT_FALSE(doc.hasKey("maps.secret_cave.portals"));
}

TEST(WorldTomlWriterTest, OutputParses) {
	EXPECT_NO_THROW(written(worldfixtures::loadMinimalWorld()));
}

TEST(WorldTomlWriterTest, WrapsGridArraysByMapWidth) {
	const World world = worldfixtures::loadMinimalWorld();
	const std::string text = WorldTomlWriter().write(world);
	const auto farmPos = text.find("[maps.farm]");
	ASSERT_NE(farmPos, std::string::npos);
	const auto groundPos = text.find("ground = [", farmPos);
	ASSERT_NE(groundPos, std::string::npos);
	const auto groundEnd = text.find(']', groundPos);
	ASSERT_NE(groundEnd, std::string::npos);
	const std::string groundBlock =
		text.substr(groundPos, groundEnd - groundPos + 1);
	int newlines = 0;
	for (const char ch : groundBlock) {
		if (ch == '\n') {
			++newlines;
		}
	}
	const int height = world.maps.at("farm").height;
	// 1 newline after '[' + height lines for rows.
	EXPECT_EQ(newlines, height + 1);
}

}
