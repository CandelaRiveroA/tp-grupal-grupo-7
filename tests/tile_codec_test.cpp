#include <optional>
#include <string>

#include "gtest/gtest.h"
#include "world_model/FormatError.h"
#include "world_model/TileCodec.h"

namespace {

using worldmodel::FormatError;
using worldmodel::FormatErrorKind;
using worldmodel::TileCodec;
using worldmodel::TileRef;

std::optional<TileRef> decode(const std::string& text) {
	return TileCodec::decode(text, "tile");
}

void expectInvalid(const std::string& text) {
	try {
		(void)decode(text);
		FAIL() << "expected FormatError for \"" << text << "\"";
	} catch (const FormatError& error) {
		EXPECT_EQ(error.kind(), FormatErrorKind::InvalidValue);
		EXPECT_EQ(error.path(), "tile");
	}
}

TEST(TileCodecTest, DecodeValidTile) {
	const std::optional<TileRef> tile = decode("outdoors:3,2");
	ASSERT_TRUE(tile.has_value());
	EXPECT_EQ(tile->tileset, "outdoors");
	EXPECT_EQ(tile->column, 3);
	EXPECT_EQ(tile->row, 2);
}

TEST(TileCodecTest, DecodeEmptyGivesNullopt) {
	EXPECT_FALSE(decode("").has_value());
}

TEST(TileCodecTest, DecodeInvalidSyntaxThrows) {
	expectInvalid("outdoors:x,0");
	expectInvalid("outdoors:0");
	expectInvalid("outdoors:0,0,0");
	expectInvalid("outdoors:0,0:1");
	expectInvalid("outdoors:,0");
	expectInvalid(":0,0");
	expectInvalid("outdoors: 0,0");
	expectInvalid("outdoors:-1,0");
}

TEST(TileCodecTest, EncodeNulloptIsEmpty) {
	EXPECT_EQ(TileCodec::encode(std::nullopt), "");
}

TEST(TileCodecTest, EncodeRoundTrip) {
	const TileRef tile{"outdoors", 3, 2};
	EXPECT_EQ(TileCodec::encode(tile), "outdoors:3,2");
	EXPECT_EQ(decode(TileCodec::encode(tile)), tile);
}

}
