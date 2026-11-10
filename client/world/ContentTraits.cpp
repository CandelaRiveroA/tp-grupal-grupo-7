#include "world/ContentTraits.h"

namespace world {

void ContentTraits::set(const std::string& contentId,
			ContentTrait trait) {
	traits_[contentId] = trait;
}

const ContentTrait* ContentTraits::find(
	const std::string& contentId) const {
	const auto it = traits_.find(contentId);
	return it == traits_.end() ? nullptr : &it->second;
}

bool ContentTraits::isDroppable(const std::string& contentId) const {
	const ContentTrait* trait = find(contentId);
	return trait != nullptr && trait->droppable;
}

}  // namespace world
