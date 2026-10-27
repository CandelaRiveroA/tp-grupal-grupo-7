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
	struct DirectionData {
		std::map<ActionKind, ActionData> actions;
		bool mirrored = false;
	};

	struct CreatureEntry {
		std::string sheetFile;
		int sheetCellWidth = 0;
		int sheetCellHeight = 0;
		int drawnWidth = 0;
		int drawnHeight = 0;
		MoveCadence cadence{0, 0};

		std::array<DirectionData, 4> directionData;
		std::array<DirectionAppearance, 4> appearance;
		CreatureData data{nullptr, {0, 0}};
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

	void relink(CreatureEntry& entry) const;

	std::vector<std::unique_ptr<CreatureEntry>> entries_;
};

}
