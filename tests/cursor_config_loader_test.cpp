#include <string>

#include "gtest/gtest.h"

#include "world/ConfigError.h"
#include "world/CursorCatalog.h"
#include "world/CursorConfigLoader.h"

namespace {

using world::ConfigError;
using world::ConfigErrorKind;
using world::CursorCatalog;
using world::CursorConfigLoader;
using world::CursorKind;

const char* const kCursorsToml = R"(
[cursor]
hotspot = [0, 0]

[normal]
sheet = "ui/cursors.png"
cell  = [16, 16]
at    = [0, 0]

[interactive]
sheet = "ui/cursors.png"
cell  = [16, 16]
at    = [2, 0]
)";

TEST(CursorConfigLoaderTest, LoadsBothCursorVariants) {
	const CursorCatalog catalog =
			CursorConfigLoader().loadFromString(kCursorsToml);

	const world::SpriteVisual* normal = catalog.find(CursorKind::Normal);
	ASSERT_NE(normal, nullptr);
	EXPECT_EQ(normal->sheetImage, "ui/cursors.png");
	EXPECT_EQ(normal->cellWidth, 16);
	EXPECT_EQ(normal->cellHeight, 16);
	EXPECT_EQ(normal->col, 0);
	EXPECT_EQ(normal->row, 0);

	const world::SpriteVisual* interactive =
			catalog.find(CursorKind::Interactive);
	ASSERT_NE(interactive, nullptr);
	EXPECT_EQ(interactive->col, 2);
	EXPECT_EQ(interactive->row, 0);

	EXPECT_EQ(catalog.hotspot.x, 0);
	EXPECT_EQ(catalog.hotspot.y, 0);
}

TEST(CursorConfigLoaderTest, ZeroCellIsInvalidValue) {
	const std::string toml = R"(
[cursor]
hotspot = [0, 0]

[normal]
sheet = "ui/cursors.png"
cell  = [0, 16]
at    = [0, 0]

[interactive]
sheet = "ui/cursors.png"
cell  = [16, 16]
at    = [2, 0]
)";
	try {
		CursorConfigLoader().loadFromString(toml);
		FAIL() << "se esperaba ConfigError";
	} catch (const ConfigError& error) {
		EXPECT_EQ(error.kind(), ConfigErrorKind::InvalidValue);
	}
}

TEST(CursorConfigLoaderTest, NegativeHotspotIsInvalidValue) {
	const std::string toml = R"(
[cursor]
hotspot = [-1, 0]

[normal]
sheet = "ui/cursors.png"
cell  = [16, 16]
at    = [0, 0]

[interactive]
sheet = "ui/cursors.png"
cell  = [16, 16]
at    = [2, 0]
)";
	try {
		CursorConfigLoader().loadFromString(toml);
		FAIL() << "se esperaba ConfigError";
	} catch (const ConfigError& error) {
		EXPECT_EQ(error.kind(), ConfigErrorKind::InvalidValue);
	}
}

TEST(CursorConfigLoaderTest, MissingVariantThrowsConfigError) {
	// Sin la tabla [interactive] falta su sheet: el TOML es inválido.
	const std::string toml = R"(
[cursor]
hotspot = [0, 0]

[normal]
sheet = "ui/cursors.png"
cell  = [16, 16]
at    = [0, 0]
)";
	EXPECT_THROW(CursorConfigLoader().loadFromString(toml), ConfigError);
}

}  // namespace
