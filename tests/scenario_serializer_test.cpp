#include <cassert>
#include <optional>
#include <string>
#include <vector>

#include "gtest/gtest.h"
#include "world_model/FormatError.h"
#include "world_model/ScenarioSerializer.h"

namespace {

using worldmodel::FormatError;
using worldmodel::FormatErrorKind;
using worldmodel::MapKind;
using worldmodel::Scenario;
using worldmodel::ScenarioSerializer;
using worldmodel::TileGrid;
using worldmodel::TileRef;
using worldmodel::TilesetDef;

const std::string kSmallScenario = R"(format_version = 1

[scenario]
id = "farm_a"
kind = "farm"
width = 2
height = 1
tile_size = 16
collision = [false, true]
ground = ["outdoors:0,0", ""]

[tilesets.outdoors]
image = "a.png"
cell = [16, 16]
)";

std::string replaced(std::string text, const std::string& from,
		     const std::string& to) {
	const std::size_t pos = text.find(from);
	assert(pos != std::string::npos);
	text.replace(pos, from.size(), to);
	return text;
}

void expectError(const std::string& toml, FormatErrorKind kind,
		 const std::string& path) {
	try {
		(void)ScenarioSerializer().loadFromString(toml);
		FAIL() << "expected FormatError at " << path;
	} catch (const FormatError& error) {
		EXPECT_EQ(error.kind(), kind) << error.what();
		EXPECT_EQ(error.path(), path);
	}
}

TEST(ScenarioSerializerTest, RoundTripPreservesScenario) {
	Scenario scenario;
	scenario.id = "farm_spring";
	scenario.kind = MapKind::Farm;
	scenario.width = 4;
	scenario.height = 3;
	scenario.tileSize = 16;
	scenario.tilesets.emplace("outdoors",
				  TilesetDef{"outdoors.png", 16, 16});
	scenario.collision.assign(12, false);
	scenario.collision[0] = true;
	scenario.collision[11] = true;
	scenario.ground = TileGrid(12);
	scenario.ground[0] = TileRef{"outdoors", 1, 2};
	scenario.ground[7] = TileRef{"outdoors", 3, 4};

	const ScenarioSerializer serializer;
	const Scenario loaded =
		serializer.loadFromString(serializer.saveToString(scenario));

	EXPECT_EQ(loaded.id, scenario.id);
	EXPECT_EQ(loaded.kind, scenario.kind);
	EXPECT_EQ(loaded.width, scenario.width);
	EXPECT_EQ(loaded.height, scenario.height);
	EXPECT_EQ(loaded.tileSize, scenario.tileSize);
	EXPECT_EQ(loaded.collision, scenario.collision);
	EXPECT_EQ(loaded.ground, scenario.ground);
	ASSERT_EQ(loaded.tilesets.size(), 1u);
	EXPECT_EQ(loaded.tilesets.at("outdoors").image, "outdoors.png");
	EXPECT_EQ(loaded.tilesets.at("outdoors").cellWidth, 16);
	EXPECT_EQ(loaded.tilesets.at("outdoors").cellHeight, 16);
}

TEST(ScenarioSerializerTest, WritesFormatHeaders) {
	const Scenario scenario =
		ScenarioSerializer().loadFromString(kSmallScenario);
	const std::string text = ScenarioSerializer().saveToString(scenario);
	EXPECT_NE(text.find("format_version = 1"), std::string::npos);
	EXPECT_NE(text.find("[scenario]"), std::string::npos);
	EXPECT_NE(text.find("[tilesets.outdoors]"), std::string::npos);
}

TEST(ScenarioSerializerTest, UnsupportedVersionIsInvalid) {
	expectError(replaced(kSmallScenario, "format_version = 1",
			     "format_version = 2"),
		    FormatErrorKind::InvalidValue, "format_version");
}

TEST(ScenarioSerializerTest, UnknownKindReportsKindPath) {
	expectError(replaced(kSmallScenario, "kind = \"farm\"",
			     "kind = \"nope\""),
		    FormatErrorKind::InvalidValue, "scenario.kind");
}

TEST(ScenarioSerializerTest, InvalidEncodingReportsCellPath) {
	for (const char* encoding : {"outdoors:x,0", "outdoors:0",
				     "outdoors:0,0,0"}) {
		const std::string toml = replaced(
			kSmallScenario, "ground = [\"outdoors:0,0\", \"\"]",
			std::string("ground = [\"") + encoding + "\", \"\"]");
		expectError(toml, FormatErrorKind::InvalidValue,
			    "scenario.ground[0]");
	}
}

TEST(ScenarioSerializerTest, UnknownTilesetReportsCellPath) {
	expectError(replaced(kSmallScenario, "\"outdoors:0,0\"",
			     "\"nope:0,0\""),
		    FormatErrorKind::InvalidValue, "scenario.ground[0]");
}

TEST(ScenarioSerializerTest, SizeMismatchReportsPath) {
	expectError(replaced(kSmallScenario, "collision = [false, true]",
			     "collision = [false]"),
		    FormatErrorKind::InvalidValue, "scenario.collision");
	expectError(replaced(kSmallScenario,
			     "ground = [\"outdoors:0,0\", \"\"]",
			     "ground = [\"outdoors:0,0\"]"),
		    FormatErrorKind::InvalidValue, "scenario.ground");
}

TEST(ScenarioSerializerTest, NonPositiveSizeIsInvalid) {
	expectError(replaced(kSmallScenario, "width = 2", "width = 0"),
		    FormatErrorKind::InvalidValue, "scenario.width");
}

}
