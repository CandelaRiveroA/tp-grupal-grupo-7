#pragma once

#include <map>
#include <string>

#include "world/CharacterAppearance.h"

namespace world {

// Vocabulario compartido de los nombres de acción en los TOML
// (characters.toml, tools_overlay.toml): UNA sola tabla para que los
// archivos de datos hablen el mismo idioma (convención §9: centralizar,
// no duplicar la tabla por reader).
class ActionNames {
public:
	ActionNames();

	// nullptr si el nombre no es una acción conocida.
	const ActionKind* find(const std::string& name) const;

	// Búsqueda inversa: nullptr si la acción no tiene nombre TOML.
	const std::string* nameFor(ActionKind action) const;

	// Todas las acciones conocidas, por nombre TOML.
	const std::map<std::string, ActionKind>& entries() const;

private:
	std::map<std::string, ActionKind> names_;
};

}  // namespace world
