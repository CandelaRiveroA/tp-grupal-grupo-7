#include "gtest/gtest.h"

#include <vector>

#include "world/ConfigError.h"
#include "world/DigitCatalog.h"
#include "world/DigitConfigLoader.h"

namespace {

const std::string kDigitsToml = R"(
[sheet]
image = "ui/digits.png"
cell  = [16, 16]
[digit.0]
at = [0, 0]
[digit.1]
at = [1, 0]
[digit.2]
at = [2, 0]
[digit.3]
at = [3, 0]
[digit.4]
at = [4, 0]
[digit.5]
at = [5, 0]
[digit.6]
at = [6, 0]
[digit.7]
at = [7, 0]
[digit.8]
at = [8, 0]
[digit.9]
at = [9, 0]
)";

world::DigitCatalog loadString(const std::string& toml) {
	world::DigitConfigLoader loader;
	return loader.loadFromString(toml);
}

TEST(DigitConfigLoaderTest, LoadsSheetAndCell) {
	const world::DigitCatalog catalog = loadString(kDigitsToml);
	EXPECT_EQ(catalog.sheetImage, "ui/digits.png");
	EXPECT_EQ(catalog.cellWidth, 16);
	EXPECT_EQ(catalog.cellHeight, 16);
}

TEST(DigitConfigLoaderTest, LoadsAllTenDigitsInRowZero) {
	const world::DigitCatalog catalog = loadString(kDigitsToml);
	for (int d = 0; d <= 9; ++d) {
		const world::DigitCell& cell = catalog.cellFor(d);
		EXPECT_EQ(cell.col, d);
		EXPECT_EQ(cell.row, 0);
	}
}

TEST(DigitConfigLoaderTest, MissingDigitThrows) {
	const std::string toml = R"(
[sheet]
image = "ui/digits.png"
cell  = [16, 16]
[digit.0]
at = [0, 0]
)";
	EXPECT_THROW(loadString(toml), world::ConfigError);
}

TEST(DigitConfigLoaderTest, NegativeCellIsInvalidValue) {
	const std::string toml = R"(
[sheet]
image = "ui/digits.png"
cell  = [16, 16]
[digit.0]
at = [-1, 0]
[digit.1]
at = [1, 0]
[digit.2]
at = [2, 0]
[digit.3]
at = [3, 0]
[digit.4]
at = [4, 0]
[digit.5]
at = [5, 0]
[digit.6]
at = [6, 0]
[digit.7]
at = [7, 0]
[digit.8]
at = [8, 0]
[digit.9]
at = [9, 0]
)";
	try {
		loadString(toml);
		FAIL() << "se esperaba ConfigError";
	} catch (const world::ConfigError& error) {
		EXPECT_EQ(error.kind(), world::ConfigErrorKind::InvalidValue);
	}
}

TEST(DigitsOfTest, DecomposesMultiDigitNumber) {
	EXPECT_EQ(world::digitsOf(21), (std::vector<int>{2, 1}));
}

TEST(DigitsOfTest, SingleDigit) {
	EXPECT_EQ(world::digitsOf(5), (std::vector<int>{5}));
}

TEST(DigitsOfTest, ZeroAndNegativeGiveZero) {
	EXPECT_EQ(world::digitsOf(0), (std::vector<int>{0}));
	EXPECT_EQ(world::digitsOf(-7), (std::vector<int>{0}));
}

TEST(DigitsOfTest, FiftyIsTwoDigits) {
	EXPECT_EQ(world::digitsOf(50), (std::vector<int>{5, 0}));
}

}  // namespace
