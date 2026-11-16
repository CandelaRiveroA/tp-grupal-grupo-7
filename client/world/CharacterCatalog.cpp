#include "world/CharacterCatalog.h"

#include <cstddef>
#include <utility>

namespace world {

std::size_t CharacterCatalog::indexOf(CreatureKind kind) const {
	return static_cast<std::size_t>(kind);
}

void CharacterCatalog::setEntry(CreatureKind kind, CreatureEntry entry) {
	const std::size_t index = indexOf(kind);
	if (index >= entries_.size()) {
		entries_.resize(index + 1);
	}
	entries_[index] =
		std::make_unique<CreatureEntry>(std::move(entry));
}

const CreatureData* CharacterCatalog::find(CreatureKind kind) const {
	const std::size_t index = indexOf(kind);
	if (index >= entries_.size() || entries_[index] == nullptr) {
		return nullptr;
	}
	return &entries_[index]->data;
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

bool CharacterCatalog::hasAppearance(CreatureKind kind) const {
	return entryFor(kind).appearance.present;
}

const AppearanceData& CharacterCatalog::appearanceFor(
	CreatureKind kind) const {
	return entryFor(kind).appearance;
}

int CharacterCatalog::frameCols(CreatureKind kind) const {
	return entryFor(kind).frameCols;
}

int CharacterCatalog::frameRows(CreatureKind kind) const {
	return entryFor(kind).frameRows;
}

}
