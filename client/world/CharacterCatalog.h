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
		// Tamaño en celdas del frame de referencia (0 = sin
		// referencia: cada frame se escala por sus propias celdas).
		int frameCols = 0;
		int frameRows = 0;

		CreatureData data;
		AppearanceData appearance;
	};

	CharacterCatalog() = default;

	void setEntry(CreatureKind kind, CreatureEntry entry);

	// Una sola búsqueda: nullptr si la criatura no está en el catálogo.
	const CreatureData* find(CreatureKind kind) const;

	const CreatureData& dataFor(CreatureKind kind) const;

	const std::string& sheetFile(CreatureKind kind) const;
	int sheetCellWidth(CreatureKind kind) const;
	int sheetCellHeight(CreatureKind kind) const;
	int drawnWidth(CreatureKind kind) const;
	int drawnHeight(CreatureKind kind) const;
	int frameCols(CreatureKind kind) const;
	int frameRows(CreatureKind kind) const;

	bool hasAppearance(CreatureKind kind) const;
	const AppearanceData& appearanceFor(CreatureKind kind) const;

private:
	std::size_t indexOf(CreatureKind kind) const;
	const CreatureEntry& entryFor(CreatureKind kind) const;

	std::vector<std::unique_ptr<CreatureEntry>> entries_;
};

}
