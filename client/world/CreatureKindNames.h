#pragma once

#include <map>
#include <optional>
#include <string>

#include "world/CreatureCatalog.h"

namespace world {

// Vocabulario compartido de criaturas: traduce el nombre en TOML ("orc",
// "skeleton", "chicken", ...) a world::CreatureKind. UNA sola tabla
// (convenciones.md §4/§9) usada por CharacterTomlReader y CommunityCatalog.
class CreatureKindNames {
public:
	CreatureKindNames();

	std::optional<CreatureKind> find(const std::string& name) const;
	const std::map<std::string, CreatureKind>& entries() const;

private:
	std::map<std::string, CreatureKind> creatures_;
};

}  // namespace world
