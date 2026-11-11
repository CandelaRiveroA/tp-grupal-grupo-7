#pragma once

#include <map>
#include <optional>
#include <string>

#include "world/ItemKind.h"

namespace world {

// Vocabulario contentId de los ÍTEMS: la traducción de cada ítem al
// string genérico que usan el inventario, los ítems del piso, el panel
// de obtenidos y las claves de tabla de los TOML. UNA sola tabla
// (convención §9): antes cada capa tenía su propio switch.
//
// Los recursos NO viven acá: su identidad es el contentId mismo
// (vocabulario ABIERTO, las claves de [resources.*] en resources.toml).
class ContentIds {
public:
	ContentIds();

	// nullptr si el concepto no tiene contentId.
	const std::string* find(ItemKind kind) const;

	// Traducción inversa (contentId -> vocabulario) sobre la MISMA
	// tabla: nullopt si el contentId no pertenece a ese vocabulario.
	std::optional<ItemKind> itemKindFor(const std::string& contentId) const;

	// Todo el vocabulario (para recorrer las tablas de los TOML, cuyas
	// claves son estos mismos strings).
	const std::map<ItemKind, std::string>& itemEntries() const;

private:
	std::map<ItemKind, std::string> items_;
};

}
