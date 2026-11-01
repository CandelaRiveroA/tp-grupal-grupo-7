#pragma once

#include <map>
#include <string>

#include "world/ItemKind.h"
#include "world/ResourceCatalog.h"

namespace world {

// Vocabulario contentId del juego: la traducción de cada ítem/recurso al
// string genérico que usan el inventario, los ítems del piso, el panel
// de obtenidos y las claves de tabla de los TOML. UNA sola tabla
// (convención §9): antes cada capa tenía su propio switch.
class ContentIds {
public:
	ContentIds();

	// nullptr si el concepto no tiene contentId.
	const std::string* find(ItemKind kind) const;
	const std::string* find(ResourceKind kind) const;

	// Todo el vocabulario (para recorrer las tablas de los TOML, cuyas
	// claves son estos mismos strings).
	const std::map<ItemKind, std::string>& itemEntries() const;
	const std::map<ResourceKind, std::string>& resourceEntries() const;

private:
	std::map<ItemKind, std::string> items_;
	std::map<ResourceKind, std::string> resources_;
};

}  // namespace world
