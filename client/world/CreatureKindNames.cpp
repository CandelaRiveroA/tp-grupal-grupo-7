#include "world/CreatureKindNames.h"

namespace world {

CreatureKindNames::CreatureKindNames()
	: creatures_{
		  {"player", CreatureKind::Player},
		  {"chicken", CreatureKind::Chicken},
		  {"cow", CreatureKind::Cow},
		  {"orc", CreatureKind::Orc},
		  {"skeleton", CreatureKind::Skeleton},
		  {"vampire", CreatureKind::Vampire},
		  {"skull", CreatureKind::Skull},
		  {"skull_fire", CreatureKind::SkullFire},
		  {"skull_fire_green", CreatureKind::SkullFireGreen},
	  } {}

std::optional<CreatureKind> CreatureKindNames::find(
	const std::string& name) const {
	const auto it = creatures_.find(name);
	if (it == creatures_.end()) {
		return std::nullopt;
	}
	return it->second;
}

const std::map<std::string, CreatureKind>& CreatureKindNames::entries() const {
	return creatures_;
}

}  // namespace world
