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
	  }
	, resources_{
		  {ResourceKind::Silver, "silver"},
		  {ResourceKind::Coal, "coal"},
		  {ResourceKind::Lapis, "lapis"},
		  {ResourceKind::Gold, "gold"},
	  } {}

const std::string* ContentIds::find(const ItemKind kind) const {
	const auto it = items_.find(kind);
	return it == items_.end() ? nullptr : &it->second;
}

const std::string* ContentIds::find(const ResourceKind kind) const {
	const auto it = resources_.find(kind);
	return it == resources_.end() ? nullptr : &it->second;
}

const std::map<ItemKind, std::string>& ContentIds::itemEntries() const {
	return items_;
}

const std::map<ResourceKind, std::string>& ContentIds::resourceEntries()
	const {
	return resources_;
}

}  // namespace world
