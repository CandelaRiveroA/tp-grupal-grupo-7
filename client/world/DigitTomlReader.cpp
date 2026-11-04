#include "world/DigitTomlReader.h"

#include <array>
#include <string>

#include "world/ConfigError.h"

namespace world {

namespace {

using Kind = ConfigErrorKind;

constexpr const char* kSheetImage = "sheet.image";
constexpr const char* kSheetCell = "sheet.cell";
constexpr const char* kDigit = "digit";
constexpr const char* kAt = "at";

}  // namespace

DigitTomlReader::DigitTomlReader(const tomlcfg::TomlDocument& doc)
	: doc_(doc) {}

std::string DigitTomlReader::join(const std::string& base,
				  const std::string& key) const {
	return base + "." + key;
}

DigitCatalog DigitTomlReader::read() const {
	DigitCatalog catalog;

	catalog.sheetImage = doc_.getString(kSheetImage);
	if (catalog.sheetImage.empty()) {
		throw ConfigError(Kind::InvalidValue, kSheetImage);
	}

	const std::array<int, 2> cell = doc_.getIntArray<2>(kSheetCell);
	catalog.cellWidth = cell[0];
	catalog.cellHeight = cell[1];
	if (catalog.cellWidth <= 0 || catalog.cellHeight <= 0) {
		throw ConfigError(Kind::InvalidValue, kSheetCell);
	}

	// Cada dígito 0..9 es una subtabla [digit.<n>] con at = [col, row].
	for (int d = 0; d <= 9; ++d) {
		const std::string base = join(kDigit, std::to_string(d));
		catalog.digits[static_cast<std::size_t>(d)] =
			SheetCell(doc_, join(base, kAt));
	}

	return catalog;
}

}  // namespace world
