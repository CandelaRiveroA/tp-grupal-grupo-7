#include "world/CharacterCatalog.h"

#include <cstddef>
#include <utility>

namespace world {

std::size_t CharacterCatalog::indexOf(CreatureKind kind) const {
	return static_cast<std::size_t>(kind);
}

void CharacterCatalog::relink(CreatureEntry& entry) const {
	for (std::size_t i = 0; i < entry.directionData.size(); ++i) {
		entry.appearance[i].actions = &entry.directionData[i].actions;
		entry.appearance[i].mirrored = entry.directionData[i].mirrored;
	}
	entry.data.directions = &entry.appearance;
	entry.data.cadence = entry.cadence;
}

void CharacterCatalog::setEntry(CreatureKind kind, CreatureEntry entry) {
	const std::size_t index = indexOf(kind);
	if (index >= entries_.size()) {
		entries_.resize(index + 1);
	}
	auto owned = std::make_unique<CreatureEntry>(std::move(entry));
	relink(*owned);
	entries_[index] = std::move(owned);
}

bool CharacterCatalog::has(CreatureKind kind) const {
	const std::size_t index = indexOf(kind);
	return index < entries_.size() && entries_[index] != nullptr;
}

const CharacterCatalog::CreatureEntry& CharacterCatalog::entryFor(
	CreatureKind kind) const {
	return *entries_[indexOf(kind)];
}

const CreatureData& CharacterCatalog::dataFor(CreatureKind kind) const {
	return entryFor(kind).data;
}

const std::string& CharacterCatalog::sheetFile(CreatureKind kind) const {
	return entryFor(kind).sheetFile;
}

int CharacterCatalog::sheetCellWidth(CreatureKind kind) const {
	return entryFor(kind).sheetCellWidth;
}

int CharacterCatalog::sheetCellHeight(CreatureKind kind) const {
	return entryFor(kind).sheetCellHeight;
}

int CharacterCatalog::drawnWidth(CreatureKind kind) const {
	return entryFor(kind).drawnWidth;
}

int CharacterCatalog::drawnHeight(CreatureKind kind) const {
	return entryFor(kind).drawnHeight;
}

}
