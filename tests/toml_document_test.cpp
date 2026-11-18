#include <algorithm>
#include <array>
#include <string>
#include <vector>

#include "gtest/gtest.h"
#include "toml/TomlDocument.h"

namespace {

const std::string kSampleToml = R"(
title = "demo"
count = 7
size  = [16, 32]

[owner]
name = "ana"
level = 3

[[items]]
id = 1
[[items]]
id = 2
)";

tomlcfg::TomlDocument parse(const std::string& text) {
	return tomlcfg::TomlDocument(text, tomlcfg::TomlDocument::Source::Text);
}

TEST(TomlDocumentTest, GetIntTopLevel) {
	const tomlcfg::TomlDocument doc = parse(kSampleToml);
	EXPECT_EQ(doc.getInt("count"), 7);
}

TEST(TomlDocumentTest, GetStringNested) {
	const tomlcfg::TomlDocument doc = parse(kSampleToml);
	EXPECT_EQ(doc.getString("owner.name"), "ana");
	EXPECT_EQ(doc.getInt("owner.level"), 3);
}

TEST(TomlDocumentTest, GetIntArrayFixedLength) {
	const tomlcfg::TomlDocument doc = parse(kSampleToml);
	const std::array<int, 2> size = doc.getIntArray<2>("size");
	EXPECT_EQ(size[0], 16);
	EXPECT_EQ(size[1], 32);
}

TEST(TomlDocumentTest, GetStringArray) {
	const tomlcfg::TomlDocument doc =
		parse("sounds = [\"chicken_cluck\", \"rooster\"]");
	const std::vector<std::string> sounds = doc.getStringArray("sounds");
	ASSERT_EQ(sounds.size(), 2u);
	EXPECT_EQ(sounds[0], "chicken_cluck");
	EXPECT_EQ(sounds[1], "rooster");
}

TEST(TomlDocumentTest, StringArrayMissingKeyThrows) {
	const tomlcfg::TomlDocument doc = parse(kSampleToml);
	EXPECT_THROW(doc.getStringArray("nope"), tomlcfg::TomlError);
}

TEST(TomlDocumentTest, StringArrayWrongTypeThrows) {
	const tomlcfg::TomlDocument doc = parse("count = 7");
	EXPECT_THROW(doc.getStringArray("count"), tomlcfg::TomlError);
}

TEST(TomlDocumentTest, StringArrayWithNonStringElementThrows) {
	const tomlcfg::TomlDocument doc = parse("mixed = [\"a\", 3]");
	EXPECT_THROW(doc.getStringArray("mixed"), tomlcfg::TomlError);
}

TEST(TomlDocumentTest, HasKey) {
	const tomlcfg::TomlDocument doc = parse(kSampleToml);
	EXPECT_TRUE(doc.hasKey("owner.name"));
	EXPECT_FALSE(doc.hasKey("owner.missing"));
}

TEST(TomlDocumentTest, SubtableKeysListsDirectChildren) {
	const std::string toml = R"(
[sheets.a]
x = 1
[sheets.b]
x = 2
)";
	const tomlcfg::TomlDocument doc = parse(toml);
	std::vector<std::string> keys = doc.subtableKeys("sheets");
	std::sort(keys.begin(), keys.end());
	ASSERT_EQ(keys.size(), 2u);
	EXPECT_EQ(keys[0], "a");
	EXPECT_EQ(keys[1], "b");
}

TEST(TomlDocumentTest, GetSubtableAllowsFurtherQueries) {
	const tomlcfg::TomlDocument doc = parse(kSampleToml);
	const tomlcfg::TomlDocument owner = doc.getSubtable("owner");
	EXPECT_EQ(owner.getString("name"), "ana");
}

TEST(TomlDocumentTest, GetTableArrayReturnsEachElement) {
	const tomlcfg::TomlDocument doc = parse(kSampleToml);
	const std::vector<tomlcfg::TomlDocument> items =
		doc.getTableArray("items");
	ASSERT_EQ(items.size(), 2u);
	EXPECT_EQ(items[0].getInt("id"), 1);
	EXPECT_EQ(items[1].getInt("id"), 2);
}

TEST(TomlDocumentTest, MissingKeyThrowsMissingKind) {
	const tomlcfg::TomlDocument doc = parse(kSampleToml);
	try {
		doc.getInt("nope");
		FAIL() << "se esperaba TomlError";
	} catch (const tomlcfg::TomlError& error) {
		EXPECT_EQ(error.kind(), tomlcfg::TomlErrorKind::MissingKey);
	}
}

TEST(TomlDocumentTest, WrongTypeThrowsWrongTypeKind) {
	const tomlcfg::TomlDocument doc = parse(kSampleToml);
	try {
		doc.getInt("title");
		FAIL() << "se esperaba TomlError";
	} catch (const tomlcfg::TomlError& error) {
		EXPECT_EQ(error.kind(), tomlcfg::TomlErrorKind::WrongType);
	}
}

TEST(TomlDocumentTest, IntArrayWrongLengthThrows) {
	const tomlcfg::TomlDocument doc = parse(kSampleToml);
	EXPECT_THROW(doc.getIntArray<3>("size"), tomlcfg::TomlError);
}

TEST(TomlDocumentTest, InvalidTomlThrowsParseError) {
	try {
		parse("not = = toml");
		FAIL() << "se esperaba TomlError";
	} catch (const tomlcfg::TomlError& error) {
		EXPECT_EQ(error.kind(), tomlcfg::TomlErrorKind::ParseError);
	}
}

TEST(TomlDocumentTest, IntArrayReturnsVariableLengthVector) {
	const tomlcfg::TomlDocument doc = parse("tiles = [1, 2, 3]");
	const std::vector<int> values = doc.getIntArray("tiles");
	ASSERT_EQ(values.size(), 3u);
	EXPECT_EQ(values[0], 1);
	EXPECT_EQ(values[1], 2);
	EXPECT_EQ(values[2], 3);
}

TEST(TomlDocumentTest, IntArrayEmptyReturnsEmptyVector) {
	const tomlcfg::TomlDocument doc = parse("tiles = []");
	EXPECT_TRUE(doc.getIntArray("tiles").empty());
}

TEST(TomlDocumentTest, BoolArrayReturnsBooleans) {
	const tomlcfg::TomlDocument doc = parse("blocked = [true, false, true]");
	const std::vector<bool> values = doc.getBoolArray("blocked");
	ASSERT_EQ(values.size(), 3u);
	EXPECT_TRUE(values[0]);
	EXPECT_FALSE(values[1]);
	EXPECT_TRUE(values[2]);
}

TEST(TomlDocumentTest, IntArrayWithStringElementThrowsWrongType) {
	const tomlcfg::TomlDocument doc = parse("mixed = [\"a\", 3]");
	try {
		doc.getIntArray("mixed");
		FAIL() << "se esperaba TomlError";
	} catch (const tomlcfg::TomlError& error) {
		EXPECT_EQ(error.kind(), tomlcfg::TomlErrorKind::WrongType);
	}
}

TEST(TomlDocumentTest, IntArrayWithDoubleElementThrowsWrongType) {
	const tomlcfg::TomlDocument doc = parse("mixed = [1.5, 3]");
	try {
		doc.getIntArray("mixed");
		FAIL() << "se esperaba TomlError";
	} catch (const tomlcfg::TomlError& error) {
		EXPECT_EQ(error.kind(), tomlcfg::TomlErrorKind::WrongType);
	}
}

TEST(TomlDocumentTest, IntArrayMissingKeyThrowsMissingKind) {
	const tomlcfg::TomlDocument doc = parse("tiles = [1]");
	try {
		doc.getIntArray("nope");
		FAIL() << "se esperaba TomlError";
	} catch (const tomlcfg::TomlError& error) {
		EXPECT_EQ(error.kind(), tomlcfg::TomlErrorKind::MissingKey);
	}
}

TEST(TomlDocumentTest, BoolArrayWithIntElementThrowsWrongType) {
	const tomlcfg::TomlDocument doc = parse("mixed = [true, 1]");
	try {
		doc.getBoolArray("mixed");
		FAIL() << "se esperaba TomlError";
	} catch (const tomlcfg::TomlError& error) {
		EXPECT_EQ(error.kind(), tomlcfg::TomlErrorKind::WrongType);
	}
}

TEST(TomlDocumentTest, BoolArrayMissingKeyThrowsMissingKind) {
	const tomlcfg::TomlDocument doc = parse("blocked = [true]");
	try {
		doc.getBoolArray("nope");
		FAIL() << "se esperaba TomlError";
	} catch (const tomlcfg::TomlError& error) {
		EXPECT_EQ(error.kind(), tomlcfg::TomlErrorKind::MissingKey);
	}
}

TEST(TomlDocumentTest, IntArrayOutOfRangeThrowsWrongType) {
	const tomlcfg::TomlDocument doc = parse("big = [2147483648]");
	try {
		doc.getIntArray("big");
		FAIL() << "se esperaba TomlError";
	} catch (const tomlcfg::TomlError& error) {
		EXPECT_EQ(error.kind(), tomlcfg::TomlErrorKind::WrongType);
	}
}

TEST(TomlDocumentTest, FixedIntArrayOutOfRangeThrowsWrongType) {
	const tomlcfg::TomlDocument doc = parse("big = [-2147483649]");
	try {
		doc.getIntArray<1>("big");
		FAIL() << "se esperaba TomlError";
	} catch (const tomlcfg::TomlError& error) {
		EXPECT_EQ(error.kind(), tomlcfg::TomlErrorKind::WrongType);
	}
}

}
