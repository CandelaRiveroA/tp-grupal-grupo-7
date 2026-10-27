#pragma once

#include "toml/TomlDocument.h"
#include "world/InventoryLayout.h"

namespace world {

class InventoryTomlReader {
public:
	explicit InventoryTomlReader(const tomlcfg::TomlDocument& doc);

	InventoryLayout read() const;

private:
	InventoryAnchor parseAnchor(const std::string& anchor) const;

	void readFrame(const std::string& section,
		       world::FrameCells& frame) const;

	std::string join(const std::string& section,
			 const std::string& key) const;

	SheetCell readCell(const std::string& key) const;

	void readItems(world::ItemCatalog& catalog) const;

	world::ItemKind itemKindFor(const std::string& name,
				    const std::string& context) const;

	const tomlcfg::TomlDocument& doc_;
};

}
