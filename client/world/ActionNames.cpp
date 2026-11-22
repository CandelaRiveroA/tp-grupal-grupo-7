#include "world/ActionNames.h"

namespace world {

ActionNames::ActionNames()
	: names_{
		  {"idle", ActionKind::Idle},
		  {"walk", ActionKind::Walk},
		  {"use_tool", ActionKind::UseTool},
		  {"water", ActionKind::Water},
		  {"milk", ActionKind::Milk},
		  {"pickup", ActionKind::Pickup},
		  {"carry", ActionKind::Carry},
		  {"carry_walk", ActionKind::CarryWalk},
		  {"faint", ActionKind::Faint},
		  {"attack", ActionKind::Attack},
		  {"damage", ActionKind::Damage},
		  {"death", ActionKind::Death},
		  {"sword", ActionKind::Sword},
		  {"revive", ActionKind::Revive},
	  } {}

const ActionKind* ActionNames::find(const std::string& name) const {
	const auto it = names_.find(name);
	return it == names_.end() ? nullptr : &it->second;
}

const std::string* ActionNames::nameFor(const ActionKind action) const {
	for (const auto& [name, kind] : names_) {
		if (kind == action) {
			return &name;
		}
	}
	return nullptr;
}

const std::map<std::string, ActionKind>& ActionNames::entries() const {
	return names_;
}

}  // namespace world
