#include <string>

#include "gtest/gtest.h"

#include "world/CharacterPaletteLoader.h"
#include "world/CharacterPalettes.h"
#include "world/ConfigError.h"

namespace {

using world::CharacterPaletteLoader;
using world::CharacterPalettes;
using world::ConfigError;
using world::ConfigErrorKind;

const char* const kValidToml = R"(
[[eyes]]
color = [104, 43, 15]
[[eyes]]
color = [60, 90, 200]

[[hair]]
color = [140, 90, 50]

[[pants]]
color = [30, 60, 120]
[[pants]]
color = [200, 40, 40]
)";

TEST(CharacterPaletteLoaderTest, LeeLosColoresDeOjosPeloYPantalonEnOrden) {
	const CharacterPalettes palettes =
		CharacterPaletteLoader().loadFromString(kValidToml);

	ASSERT_EQ(std::size_t{2}, palettes.eyes.size());
	ASSERT_EQ(std::size_t{1}, palettes.hair.size());
	EXPECT_EQ(104, palettes.eyes[0][0]);
	EXPECT_EQ(43, palettes.eyes[0][1]);
	EXPECT_EQ(15, palettes.eyes[0][2]);
	EXPECT_EQ(200, palettes.eyes[1][2]);
	EXPECT_EQ(140, palettes.hair[0][0]);
	ASSERT_EQ(std::size_t{2}, palettes.pants.size());
	EXPECT_EQ(30, palettes.pants[0][0]);
	EXPECT_EQ(120, palettes.pants[0][2]);
	EXPECT_EQ(200, palettes.pants[1][0]);
}

TEST(CharacterPaletteLoaderTest, UnCanalFueraDeRangoEsInvalidValueConSuPath) {
	const std::string toml = R"(
[[eyes]]
color = [104, 256, 15]
[[hair]]
color = [1, 2, 3]
[[pants]]
color = [1, 2, 3]
)";

	try {
		CharacterPaletteLoader().loadFromString(toml);
		FAIL() << "debia lanzar ConfigError";
	} catch (const ConfigError& error) {
		EXPECT_EQ(ConfigErrorKind::InvalidValue, error.kind());
		EXPECT_EQ("eyes[0].color", error.path());
	}
}

TEST(CharacterPaletteLoaderTest, SinLaPaletaDePeloLanzaConfigError) {
	const std::string toml = R"(
[[eyes]]
color = [1, 2, 3]
)";

	EXPECT_THROW(CharacterPaletteLoader().loadFromString(toml), ConfigError);
}

TEST(CharacterPaletteLoaderTest, UnColorConDosCanalesLanzaConfigError) {
	const std::string toml = R"(
[[eyes]]
color = [1, 2]
[[hair]]
color = [1, 2, 3]
[[pants]]
color = [1, 2, 3]
)";

	EXPECT_THROW(CharacterPaletteLoader().loadFromString(toml), ConfigError);
}

TEST(CharacterPaletteLoaderTest, SinLaPaletaDePantalonLanzaConfigError) {
	const std::string toml = R"(
[[eyes]]
color = [1, 2, 3]
[[hair]]
color = [1, 2, 3]
)";

	EXPECT_THROW(CharacterPaletteLoader().loadFromString(toml), ConfigError);
}

TEST(CharacterPaletteLoaderTest, UnCanalNegativoDelPantalonEsInvalidValueConSuPath) {
	const std::string toml = R"(
[[eyes]]
color = [1, 2, 3]
[[hair]]
color = [1, 2, 3]
[[pants]]
color = [1, -5, 3]
)";

	try {
		CharacterPaletteLoader().loadFromString(toml);
		FAIL() << "debia lanzar ConfigError";
	} catch (const ConfigError& error) {
		EXPECT_EQ(ConfigErrorKind::InvalidValue, error.kind());
		EXPECT_EQ("pants[0].color", error.path());
	}
}

}  // namespace
