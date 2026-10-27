#pragma once

#include <array>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include "world/CharacterAppearance.h"
#include "world/CreatureCatalog.h"

namespace world {

class CharacterCatalog {
public:
	struct CreatureEntry {
		std::string sheetFile;
		int sheetCellWidth = 0;
		int sheetCellHeight = 0;
		int drawnWidth = 0;
		int drawnHeight = 0;

		CreatureData data;
	};

	CharacterCatalog() = default;

	void setEntry(CreatureKind kind, CreatureEntry entry);

	bool has(CreatureKind kind) const;

	const CreatureData& dataFor(CreatureKind kind) const;

	const std::string& sheetFile(CreatureKind kind) const;
	int sheetCellWidth(CreatureKind kind) const;
	int sheetCellHeight(CreatureKind kind) const;
	int drawnWidth(CreatureKind kind) const;
	int drawnHeight(CreatureKind kind) const;

private:
	std::size_t indexOf(CreatureKind kind) const;
	const CreatureEntry& entryFor(CreatureKind kind) const;

	std::vector<std::unique_ptr<CreatureEntry>> entries_;
};

}
