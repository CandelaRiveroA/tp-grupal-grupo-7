#include "world/ContentIds.h"

namespace world {

ContentIds::ContentIds()
	: items_{
		  {ItemKind::Shovel, "shovel"},
		  {ItemKind::Pickaxe, "pickaxe"},
		  {ItemKind::WateringCan, "watering_can"},
		  {ItemKind::Sword, "sword"},
		  {ItemKind::Egg, "egg"},
		  {ItemKind::Milk, "milk"},
	  } {}

const std::string* ContentIds::find(const ItemKind kind) const {
	const auto it = items_.find(kind);
	return it == items_.end() ? nullptr : &it->second;
}

const std::map<ItemKind, std::string>& ContentIds::itemEntries() const {
	return items_;
}

std::optional<ItemKind> ContentIds::itemKindFor(
	const std::string& contentId) const {
	for (const auto& [kind, id] : items_) {
		if (id == contentId) {
			return kind;
		}
	}
	return std::nullopt;
}

}  // namespace world
