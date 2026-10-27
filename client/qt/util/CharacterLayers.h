#pragma once

#include <cstdint>
#include <map>

#include "common/model/draft/CharacterAppearance.h"
#include "CharacterLayer.h"

// Traduce entre una capa y el campo de CharacterAppearance que la define.
// Es la única definición del mapeo: la usan la pantalla y el preview.
class CharacterLayers {
public:
	CharacterLayers();

	std::uint8_t value(const CharacterAppearance& appearance,
			   CharacterLayer layer) const;
	void set(CharacterAppearance& appearance, CharacterLayer layer,
		 std::uint8_t index) const;

private:
	std::map<CharacterLayer, std::uint8_t CharacterAppearance::*> fields_;
};
