#include <string>

#include "gtest/gtest.h"
#include "toml/TomlDocument.h"
#include "world_model/FormatError.h"

namespace {

TEST(WorldFormatErrorTest, DirectConstruction) {
	const worldmodel::FormatError error(
		worldmodel::FormatErrorKind::InvalidValue,
		"maps.farm.ground[7]");
	EXPECT_EQ(error.kind(), worldmodel::FormatErrorKind::InvalidValue);
	EXPECT_EQ(error.path(), "maps.farm.ground[7]");
	EXPECT_STREQ(error.what(), "maps.farm.ground[7]");
}

TEST(WorldFormatErrorTest, MissingKeyMapsToMissingStructure) {
	const worldmodel::FormatError error(
		tomlcfg::TomlError(tomlcfg::TomlErrorKind::MissingKey, "farm.kind"));
	EXPECT_EQ(error.kind(), worldmodel::FormatErrorKind::MissingStructure);
	EXPECT_EQ(error.path(), "farm.kind");
}

TEST(WorldFormatErrorTest, WrongTypeMapsToWrongType) {
	const worldmodel::FormatError error(
		tomlcfg::TomlError(tomlcfg::TomlErrorKind::WrongType, "farm.width"));
	EXPECT_EQ(error.kind(), worldmodel::FormatErrorKind::WrongType);
	EXPECT_EQ(error.path(), "farm.width");
}

TEST(WorldFormatErrorTest, ParseErrorMapsToUnreadableInput) {
	const worldmodel::FormatError error(tomlcfg::TomlError(
		tomlcfg::TomlErrorKind::ParseError, "world.toml"));
	EXPECT_EQ(error.kind(), worldmodel::FormatErrorKind::UnreadableInput);
	EXPECT_EQ(error.path(), "world.toml");
}

}
