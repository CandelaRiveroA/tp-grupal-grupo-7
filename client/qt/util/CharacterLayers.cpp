#include "CharacterLayers.h"

CharacterLayers::CharacterLayers() : fields_() {
	fields_[CharacterLayer::Skin] = &CharacterAppearance::skin;
	fields_[CharacterLayer::Eyes] = &CharacterAppearance::eyes;
	fields_[CharacterLayer::Hair] = &CharacterAppearance::hair;
	fields_[CharacterLayer::HairColor] = &CharacterAppearance::hairColor;
	fields_[CharacterLayer::Shirt] = &CharacterAppearance::shirt;
	fields_[CharacterLayer::Pants] = &CharacterAppearance::pants;
	fields_[CharacterLayer::PantsColor] = &CharacterAppearance::pantsColor;
	fields_[CharacterLayer::Accessory] = &CharacterAppearance::accessory;
}

std::uint8_t CharacterLayers::value(const CharacterAppearance& appearance,
				    CharacterLayer layer) const {
	const auto field = fields_.find(layer);
	if (field == fields_.end()) {
		return 0;
	}
	return appearance.*(field->second);
}

void CharacterLayers::set(CharacterAppearance& appearance, CharacterLayer layer,
			  std::uint8_t index) const {
	const auto field = fields_.find(layer);
	if (field == fields_.end()) {
		return;
	}
	appearance.*(field->second) = index;
}
